//=============================================================================
//
// 草処理 [grass.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "grass.h"
#include "manager.h"
#include "game.h"
#include "easing.h"
#include "player.h"
#include "enemy.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CGrassBlock::CGrassBlock(int nPriority) : CBlock(nPriority)
{
	// 値のクリア
	m_rotVel = INIT_VEC3;
	m_distMax = 40.0f;
}
//=============================================================================
// デストラクタ
//=============================================================================
CGrassBlock::~CGrassBlock()
{
	// なし
}
//=============================================================================
// 更新処理
//=============================================================================
void CGrassBlock::Update(void)
{
	// ブロックの更新処理
	CBlock::Update();

	D3DXVECTOR3 thisPos = GetPos();
	float fMaxTilt = D3DXToRadian(60);

	// === プレイヤーと敵の中で一番近いものを探す ===
	float nearestDist = FLT_MAX;
	D3DXVECTOR3 nearestDiff(0, 0, 0);

	CPlayer* pPlayer = CGame::GetPlayer();

	// --- プレイヤー ---
	if (pPlayer)
	{
		D3DXVECTOR3 diff = CGame::GetPlayer()->GetPos() - thisPos;
		float dist = D3DXVec3Length(&diff);
		if (dist < nearestDist)
		{
			nearestDist = dist;
			nearestDiff = diff;
		}
	}

	// --- 敵 ---
	if (CGame::GetEnemy())
	{
		D3DXVECTOR3 diff = CGame::GetEnemy()->GetPos() - thisPos;
		float dist = D3DXVec3Length(&diff);
		if (dist < nearestDist)
		{
			nearestDist = dist;
			nearestDiff = diff;
		}
	}

	// === 草の傾き計算 ===
	D3DXVECTOR3 rot = GetRot();
	D3DXVECTOR3 targetRot(0, 0, 0);

	if (nearestDist < m_distMax)
	{
		// 正規化
		D3DXVec3Normalize(&nearestDiff, &nearestDiff);
		
		// 距離に応じた割合
		float t = 1.0f - (nearestDist / m_distMax);

		// 傾き角度 = 最大角度 × 割合
		float tilt = fMaxTilt * t;

		rot.x = -nearestDiff.z * tilt;
		rot.z = nearestDiff.x * tilt;
	}

	if (pPlayer)
	{
		D3DXVECTOR3 diff = pPlayer->GetPos() - thisPos;
		float dist = D3DXVec3Length(&diff);
		pPlayer->SetInGrass(dist < m_distMax);
	}

	// *** バネ ＋ ダンピング ***
	float stiffness = 0.12f;		// バネの強さ（0.1～0.3ぐらい）
	float damping = 0.75f;			// 減衰率（0.8～0.98ぐらい）

	// バネ力 = (目標 - 現在) × バネ定数
	m_rotVel.x += (targetRot.x - rot.x) * stiffness;
	m_rotVel.z += (targetRot.z - rot.z) * stiffness;

	// 減衰（抵抗）
	m_rotVel.x *= damping;
	m_rotVel.z *= damping;

	// 回転に反映
	rot.x += m_rotVel.x;
	rot.z += m_rotVel.z;

	// 向きの設定
	SetRot(rot);
}
//=============================================================================
// 描画処理
//=============================================================================
void CGrassBlock::Draw(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// αブレンディングを加算合成に設定
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// αテストを有効
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);//デフォルトはfalse
	pDevice->SetRenderState(D3DRS_ALPHAREF, 0);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);//0より大きかったら描画

	// ブロックの描画
	CBlock::Draw();

	// αテストを無効に戻す
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);//デフォルトはfalse

	// αブレンディングを元に戻す
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
}
