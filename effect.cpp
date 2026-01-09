//=============================================================================
//
// エフェクト処理 [effect.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "effect.h"
#include "renderer.h"
#include "manager.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CEffect::CEffect(int nPriority) : CObjectBillboard(nPriority)
{
	// 値のクリア
	m_move			= INIT_VEC3;// 位置
	m_fRadius		= 0.0f;		// 半径
	m_nLife			= 0;		// 寿命
	m_nIdxTexture	= 0;		// テクスチャインデックス
	m_fGravity		= 0.0f;		// 重力
	m_bBlend		= true;		// アルファブレンドするかどうが
	m_bTurn			= false;	// 回転するか
	m_turnSpeed		= 0.0f;		// 回転スピード
}
//=============================================================================
// デストラクタ
//=============================================================================
CEffect::~CEffect()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CEffect* CEffect::Create(const EffectDesc& desc)
{
	// エフェクトオブジェクトの生成
	CEffect* pEffect = new CEffect;

	// nullptrだったら
	if (pEffect == nullptr)
	{
		return nullptr;
	}

	pEffect->SetPath(desc.path);

	// 初期化失敗時
	if (FAILED(pEffect->Init()))
	{
		return nullptr;
	}

	pEffect->SetPos(desc.pos);
	pEffect->SetMove(desc.move);
	pEffect->SetCol(desc.col);
	pEffect->SetRadius(desc.fRadius);
	pEffect->SetLife(desc.nLife);
	pEffect->SetGravity(desc.fGravity);
	pEffect->SetDecRadius(desc.fDecRadius);
	pEffect->SetBlend(desc.bBlend);
	pEffect->SetTurn(desc.bTurn);
	pEffect->SetTurnSpeed(desc.turnSpeed);

	return pEffect;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CEffect::Init(void)
{
	// ビルボードオブジェクトの初期化処理
	CObjectBillboard::Init();

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CEffect::Uninit(void)
{
	// ビルボードオブジェクトの終了処理
	CObjectBillboard::Uninit();
}
//=============================================================================
// 更新処理
//=============================================================================
void CEffect::Update(void)
{
	// ビルボードオブジェクトの更新処理
	CObjectBillboard::Update();

	if (m_bTurn)// 回転あり
	{
		// 向きの取得
		D3DXVECTOR3 rot = GetRot();

		// 回転させる
		rot.z += m_turnSpeed;

		// 向きの設定
		SetRot(rot);

		// ビルボードオブジェクトの更新処理(回転)
		CObjectBillboard::UpdateTurn();
	}

	m_move.y += m_fGravity; // 重力加速度

	// 位置の取得
	D3DXVECTOR3 Pos = GetPos();

	// 位置を更新
	Pos += m_move;

	// 位置の設定
	SetPos(Pos);
	SetSize(m_fRadius);

	m_fRadius -= m_fDecRadius;

	if (m_fRadius <= 0.0f)
	{
		m_fRadius = 0.0f;
	}

	m_nLife--;

	if (m_nLife <= 0 || m_fRadius <= 0.0f)
	{
		// エフェクトの終了
		Uninit();
		return;
	}
}
//=============================================================================
// 描画処理
//=============================================================================
void CEffect::Draw(void)
{
	// テクスチャの取得
	CTexture* pTexture = CManager::GetTexture();

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	if (m_bBlend)
	{
		pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);         // ライトを無効にする

		// αブレンディングを加算合成に設定
		pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

		// αテストを有効
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);//デフォルトはfalse
		pDevice->SetRenderState(D3DRS_ALPHAREF, 0);
		pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);//0より大きかったら描画
	}

	// テクスチャの設定
	pDevice->SetTexture(0, pTexture->GetAddress(m_nIdxTexture));

	// ビルボードオブジェクトの描画処理
	CObjectBillboard::Draw();

	if (m_bBlend)
	{
		// αテストを無効に戻す
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);//デフォルトはfalse

		// αブレンディングを元に戻す
		pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
		pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
		pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

		pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);          // ライトを有効にする
	}
}


//=============================================================================
// モデルエフェクトのコンストラクタ
//=============================================================================
CModelEffect::CModelEffect()
{
	// 値のクリア
	m_nLife		= 0;			// 寿命（秒）
	m_fGravity	= 0.0f;			// 重力加速度
	m_scaleDec	= 0.0f;			// サイズ減衰率
	m_velocity	= INIT_VEC3;	// 速度
}
//=============================================================================
// モデルエフェクトのデストラクタ
//=============================================================================
CModelEffect::~CModelEffect()
{
	// なし
}
//=============================================================================
// モデルエフェクトの生成処理
//=============================================================================
CModelEffect* CModelEffect::Create(const char* path, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 velocity, D3DXVECTOR3 size, 
	int nLife, float fGravity, float scaleDec)
{
	CModelEffect* pEffect = new CModelEffect;

	// nullptrだったら
	if (pEffect == nullptr)
	{
		return nullptr;
	}

	pEffect->SetPath(path);

	// 初期化失敗時
	if (FAILED(pEffect->Init()))
	{
		return nullptr;
	}

	pEffect->SetPos(pos);
	pEffect->SetRot(rot);
	pEffect->SetVelocity(velocity);
	pEffect->SetSize(size);
	pEffect->SetLife(nLife);
	pEffect->SetGravity(fGravity);
	pEffect->SetScaleDec(scaleDec);

	return pEffect;
}
//=============================================================================
// モデルエフェクトの初期化処理
//=============================================================================
HRESULT CModelEffect::Init(void)
{
	// Xファイルオブジェクトの初期化処理
	CObjectX::Init();

	return S_OK;
}
//=============================================================================
// モデルエフェクトの終了処理
//=============================================================================
void CModelEffect::Uninit(void)
{
	// Xファイルオブジェクトの終了処理
	CObjectX::Uninit();
}
//=============================================================================
// モデルエフェクトの更新処理
//=============================================================================
void CModelEffect::Update(void)
{
	D3DXVECTOR3 Pos = GetPos();		// 位置の取得
	D3DXVECTOR3 Rot = GetRot();		// 向きの取得
	D3DXVECTOR3 Size = GetSize();	// サイズの取得

	if (m_nLife <= 0 || Size.x <= 0 || Size.y <= 0 || Size.z <= 0)
	{
		// 終了処理
		Uninit();
		return;
	}

	// 寿命を減らす
	m_nLife--;

	// サイズの減衰
	Size.x -= m_scaleDec;
	Size.y -= m_scaleDec;
	Size.z -= m_scaleDec;

	// 重力を加算
	m_velocity.y += m_fGravity;

	// 位置を更新
	Pos += m_velocity;

	SetPos(Pos);
	SetRot(Rot);
	SetSize(Size);

	// Xファイルオブジェクトの更新処理
	CObjectX::Update();
}
