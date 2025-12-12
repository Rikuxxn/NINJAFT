//=============================================================================
//
// ブロックリスト処理 [blocklist.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "blocklist.h"
#include "player.h"
#include "game.h"
#include "manager.h"
#include "particle.h"
#include "algorithm"
#include "meshcylinder.h"
#include "collisionUtils.h"
#include "motion.h"
#include "time.h"

// 名前空間stdの使用
using namespace std;

//=============================================================================
// 木箱ブロックのコンストラクタ
//=============================================================================
CWoodBoxBlock::CWoodBoxBlock()
{
	// 値のクリア
	m_ResPos = INIT_VEC3;// リスポーン位置
}
//=============================================================================
// 木箱ブロックのデストラクタ
//=============================================================================
CWoodBoxBlock::~CWoodBoxBlock()
{
	// なし
}
//=============================================================================
// 木箱ブロックの初期化処理
//=============================================================================
HRESULT CWoodBoxBlock::Init(void)
{
	// ブロックの初期化処理
	CBlock::Init();

	// 最初の位置をリスポーン位置に設定
	m_ResPos = GetPos();

	// 動的に戻す
	SetEditMode(false);

	return S_OK;
}
//=============================================================================
// 木箱ブロックの更新処理
//=============================================================================
void CWoodBoxBlock::Update()
{
	CBlock::Update(); // 共通処理
}
//=============================================================================
// リスポーン処理
//=============================================================================
void CWoodBoxBlock::Respawn(D3DXVECTOR3 resPos)
{
	// 動かすためにキネマティックにする
	SetEditMode(true);

	// リスポーン位置に設定
	SetPos(resPos);
	SetRot(INIT_VEC3);

	// コライダーの更新
	UpdateCollider();

	// 動的に戻す
	SetEditMode(false);
}


//=============================================================================
// 灯籠ブロックのコンストラクタ
//=============================================================================
CTorchBlock::CTorchBlock()
{
	// 値のクリア
}
//=============================================================================
// 灯籠ブロックのデストラクタ
//=============================================================================
CTorchBlock::~CTorchBlock()
{
	// なし
}
//=============================================================================
// 灯籠ブロックの更新処理
//=============================================================================
void CTorchBlock::Update(void)
{
	// ブロックの更新処理
	CBlock::Update();

	// プレイヤーの取得
	CPlayer* pPlayer = CGame::GetPlayer();

	if (!pPlayer)
	{
		return;
	}

	// 対象との距離を求めるためにプレイヤーの位置を取得
	D3DXVECTOR3 playerPos = pPlayer->GetPos();

	// 対象との距離を求める
	D3DXVECTOR3 disPos = playerPos - GetPos();
	float distance = D3DXVec3Length(&disPos);

	// 範囲内に入ったら
	if (distance < m_distMax)
	{
		pPlayer->SetInTorch(true);
	}
}
//=============================================================================
// 灯籠ブロックのライト更新処理
//=============================================================================
void CTorchBlock::UpdateLight(void)
{
	// 時間の割合を取得
	float progress = CGame::GetTime()->GetProgress(); // 0.0～0.1

	// ======== 夜のフェード処理 ========
	float torchIntensity = 0.0f;

	// 夜になる手前（夕方）でフェードイン
	if (progress >= 0.25f && progress < 0.30f)
	{
		torchIntensity = (progress - 0.25f) / (0.30f - 0.25f); // 0→1
	}
	// 夜中は最大
	else if (progress >= 0.30f && progress < 0.90f)
	{
		torchIntensity = 1.0f;
	}
	// 明け方でフェードアウト
	else if (progress >= 0.90f && progress < 1.0f)
	{
		torchIntensity = 1.0f - (progress - 0.90f) / (1.0f - 0.90f); // 1→0
	}
	// それ以外は消灯
	else
	{
		torchIntensity = 0.0f;
	}

	// ======== 灯籠ライト ========
	if (torchIntensity > 0.0f)
	{
		D3DXCOLOR torchColor(1.0f, 0.7f, 0.3f, 0.8f);
		torchColor *= torchIntensity; // フェード強度反映

		CLight::AddLight(
			D3DLIGHT_POINT,
			torchColor,
			D3DXVECTOR3(0.0f, -1.0f, 0.0f),
			D3DXVECTOR3(GetPos().x, GetPos().y + 20.0f, GetPos().z)
		);
	}
}


//=============================================================================
// 水ブロックのコンストラクタ
//=============================================================================
CWaterBlock::CWaterBlock(int nPriority) : CBlock(nPriority)
{
	// 値のクリア
	m_counter = 0;// 生成カウンター
	m_isHit = false;// 当たっているか
}
//=============================================================================
// 水ブロックのデストラクタ
//=============================================================================
CWaterBlock::~CWaterBlock()
{
	// なし
}
//=============================================================================
// 水ブロックの更新処理
//=============================================================================
void CWaterBlock::Update(void)
{
	// ブロックの更新処理
	CBlock::Update();

	// プレイヤーの足元に波紋を生成するため、プレイヤーの取得をする
	CPlayer* pPlayer = CGame::GetPlayer();

	// 移動しているときに生成するため、入力を取得する
	InputData input = pPlayer->GatherInput();

	D3DXVECTOR3 pPos = pPlayer->GetColliderPos(); // カプセルコライダー中心位置

	// カプセルとAABBの当たり判定
	bool isOverlap = CCollision::CheckCapsuleAABBCollision(GetPos(), GetModelSize(), GetSize(),
		pPos, pPlayer->GetRadius(), pPlayer->GetHeight());

	if (isOverlap)
	{
		m_isHit = true;
	}
	else
	{
		m_isHit = false;
	}

	if (pPlayer && isOverlap && (input.moveDir.x != 0.0f || input.moveDir.z != 0.0f) &&
		!pPlayer->GetMotion()->IsCurrentMotion(CPlayer::STEALTH_MOVE))
	{
		m_counter++;

		// プレイヤーの位置
		D3DXVECTOR3 pos = pPlayer->GetPos();
		pos.y += 10.0f;

		if (m_counter >= SPAWN_TIME)
		{
			// 半径を決めてランダム位置にスポーン
			float radiusMax = SPAWN_RADIUS;

			// 0.0～1.0 の乱数
			float r = (rand() % 10000) / 10000.0f;

			// 平方根を取って均一に分布させる
			float radius = sqrtf(r) * radiusMax;

			// 角度
			float angle = ((rand() % 360) / 180.0f) * D3DX_PI;

			// 位置
			pos.x = pos.x + cosf(angle) * radius;
			pos.y += 2.0f;
			pos.z = pos.z + sinf(angle) * radius;

			// 波紋の生成
			CMeshCylinder::Create(pos, D3DXCOLOR(0.3f, 0.7f, 1.0f, 0.9f), 5.0f, 8.0f, 0.5f, 50, 0.03f);

			m_counter = 0;
		}
	}
	else
	{
		// カウンターをリセット
		m_counter = 0;
	}
}

//*****************************************************************************
// 埋蔵金ブロックの静的メンバ変数宣言
//*****************************************************************************
int CBuriedTreasureBlock::m_getCount = 0;

//=============================================================================
// 埋蔵金ブロックのコンストラクタ
//=============================================================================
CBuriedTreasureBlock::CBuriedTreasureBlock()
{
	// 値のクリア
	m_effectTimer = 0;		// エフェクト生成タイマー
	m_guageRate;			// ゲージの最大量
	m_guageDecreaseSpeed;	// ゲージの減る量
	m_bUiActive = false;	// UIが表示されているか
	m_getCount = 0;			// 取得数
}
//=============================================================================
// 埋蔵金ブロックのデストラクタ
//=============================================================================
CBuriedTreasureBlock::~CBuriedTreasureBlock()
{
	// なし
}
//=============================================================================
// 埋蔵金ブロックの初期化処理
//=============================================================================
HRESULT CBuriedTreasureBlock::Init(void)
{
	// ブロックの初期化処理
	CBlock::Init();

	// ゲージ生成（任意サイズ・色）
	m_pFrame = C3DGuage::Create(C3DGuage::TYPE_FRAME, D3DXVECTOR3(0.0f, 110.0f, 0.0f), 70.0f, 4.0f);
	m_pGuage = C3DGuage::Create(C3DGuage::TYPE_GUAGE, D3DXVECTOR3(0.0f, 110.0f, 0.0f), 70.0f, 4.0f);

	// 最初は非表示
	m_pFrame->SetActive(false);
	m_pGuage->SetActive(false);

	m_guageRate = GUAGE_RATE;
	m_guageDecreaseSpeed = GUAGE_DECREASE_SPEED; // 1フレームで減る割合

	return S_OK;
}
//=============================================================================
// 埋蔵金ブロックの更新処理
//=============================================================================
void CBuriedTreasureBlock::Update(void)
{
	// ブロックの更新処理
	CBlock::Update();

	m_effectTimer++;

	if (m_effectTimer >= SPAWN_TIME)
	{
		m_effectTimer = 0;

		// お宝エフェクトの生成
		CParticle::Create<CTreasureParticle>(INIT_VEC3, GetPos(), D3DXCOLOR(0.6f, 0.6f, 0.0f, 0.3f), 50, 10);
	}

	CPlayer* pPlayer = CGame::GetPlayer();

	if (!pPlayer)
	{
		return;
	}

	// 対象との距離を求めるためにプレイヤーの位置を取得
	D3DXVECTOR3 playerPos = pPlayer->GetPos();

	// 対象との距離を求める
	D3DXVECTOR3 disPos = playerPos - GetPos();
	float distance = D3DXVec3Length(&disPos);

	// 入力判定をするために各インプット処理を取得
	CInputMouse* pMouse = CManager::GetInputMouse();
	CInputJoypad* pJoypad = CManager::GetInputJoypad();

	// 範囲内に入ったら
	if (distance < TRIGGER_DISTANCE)
	{
		// ゲージを表示
		if (!m_bUiActive)
		{
			if (m_pFrame && m_pGuage)
			{
				m_pFrame->SetActive(true);
				m_pGuage->SetActive(true);
			}

			m_bUiActive = true; // 表示中フラグ
		}

		// ダメージ状態中または移動入力があるときはゲージを溜めれないようにする
		bool isDamage = pPlayer->GetMotion()->IsCurrentMotion(CPlayer::DAMAGE);
		bool isMoving = pPlayer->GetIsMoving();

		// ゲージを減らす
		if (!isDamage && !isMoving && 
			(pMouse->GetPress(0) || pJoypad->GetPress(CInputJoypad::JOYKEY_A)))
		{
			m_guageRate -= m_guageDecreaseSpeed;

			if (m_guageRate < 0.0f)
			{
				m_guageRate = 0.0f;
			}
		}
	}
	else
	{
		// ゲージを非表示
		if (m_bUiActive)
		{
			if (m_pFrame && m_pGuage)
			{
				m_pFrame->SetActive(false);
				m_pGuage->SetActive(false);
			}

			m_bUiActive = false; // 非表示にする
		}
	}

	// ゲージの表示更新
	if (m_pGuage)
	{
		D3DXVECTOR3 guagePos = GetPos() + D3DXVECTOR3(0, 80.0f, 0);
		m_pFrame->SetPos(guagePos);
		m_pGuage->SetPos(guagePos);

		m_pGuage->UpdateGuageVtx(m_guageRate / 100.0f);

		// フレームも更新
		m_pFrame->UpdateFrame();
	}

	// ゲージがゼロになったらブロック削除
	if (m_guageRate <= 0.0f)
	{
		// 取得カウントを増やす
		m_getCount++;

		// 埋蔵金が取られたことを通知して、リストから位置を削除する
		CGame::GetBlockManager()->OnTreasureCollected(GetPos());

		// 削除予約
		Kill();

		// ゲージも削除する
		m_pGuage->Uninit();
		m_pFrame->Uninit();
	}
}

//*****************************************************************************
// 扉ブロックの静的メンバ変数宣言
//*****************************************************************************
bool CDoorBlock::m_isOpen = false;

//=============================================================================
// 扉ブロックのコンストラクタ
//=============================================================================
CDoorBlock::CDoorBlock()
{
	// 値のクリア
	m_baseRotY = 0.0f;	// 基準の角度
	m_rotY = 0.0f;		// Y角度
}
//=============================================================================
// 扉ブロックのデストラクタ
//=============================================================================
CDoorBlock::~CDoorBlock()
{
	// なし
}
//=============================================================================
// 扉ブロックの更新処理
//=============================================================================
void CDoorBlock::Update(void)
{
	// ブロックの更新処理
	CBlock::Update();

	if (!DoorOpen())
	{
		return;
	}

	m_isOpen = true;

	// 初期角度が正なら -90°回転、負なら +90°回転
	float targetRotY = m_baseRotY + ((m_baseRotY >= 0.0f) ? -ROT_LIMIT : +ROT_LIMIT);

	// 補間処理
	if (m_rotY < targetRotY)
	{
		m_rotY += ROT_SPEED;

		if (m_rotY > targetRotY)
		{
			m_rotY = targetRotY;
		}
	}
	else if (m_rotY > targetRotY)
	{
		m_rotY -= ROT_SPEED;

		if (m_rotY < targetRotY)
		{
			m_rotY = targetRotY;
		}
	}

	// 回転を適用
	D3DXVECTOR3 rot = GetRot();
	rot.y = D3DXToRadian(m_rotY);
	SetRot(rot);
}


//=============================================================================
// 出口判定ブロックのコンストラクタ
//=============================================================================
CExitBlock::CExitBlock(int nPriority) : CBlock(nPriority)
{
	// ゴーストオブジェクト化
	SetGhostObject(true);

	// 値のクリア
	m_isEscape = false;
}
//=============================================================================
// 出口判定ブロックのデストラクタ
//=============================================================================
CExitBlock::~CExitBlock()
{
	// なし
}
//=============================================================================
// 出口判定ブロックの更新処理
//=============================================================================
void CExitBlock::Update(void)
{
	// ブロックの更新処理
	CBlock::Update();

	CPlayer* pPlayer = CGame::GetPlayer();

	if (!pPlayer)
	{
		return;
	}

	// 脱出可能だったら
	if (AvailableExit())
	{
		// 対象との距離を求めるためにプレイヤーの位置を取得
		D3DXVECTOR3 playerPos = pPlayer->GetPos();

		// 対象との距離を求める
		D3DXVECTOR3 disPos = playerPos - GetPos();
		float distance = D3DXVec3Length(&disPos);

		// 範囲内だったら
		if (distance < TRIGGER_DISTACE)
		{
			m_isEscape = true;
		}
	}
}
//=============================================================================
// 接触判定処理
//=============================================================================
bool CExitBlock::IsHitPlayer(CPlayer* pPlayer)
{
	//=====================================================================
	// プレスブロック側（OBB）
	//=====================================================================
	OBB obb;
	obb.center = GetPos();

	D3DXMATRIX world = GetWorldMatrix();
	obb.axis[0] = D3DXVECTOR3(world._11, world._12, world._13);
	obb.axis[1] = D3DXVECTOR3(world._21, world._22, world._23);
	obb.axis[2] = D3DXVECTOR3(world._31, world._32, world._33);

	// 軸は念のため正規化
	for (int i = 0; i < 3; i++)
	{
		D3DXVec3Normalize(&obb.axis[i], &obb.axis[i]);
	}

	// モデルサイズとスケール適用
	D3DXVECTOR3 modelSize = GetModelSize();
	D3DXVECTOR3 scale = GetSize();
	obb.halfSize.x = (modelSize.x * scale.x) * 0.5f;
	obb.halfSize.y = (modelSize.y * scale.y) * 0.5f;
	obb.halfSize.z = (modelSize.z * scale.z) * 0.5f;

	//=========================================================================
	// プレイヤー側（AABB）
	//=========================================================================

	// カプセルコライダーのサイズからAABBサイズを計算
	D3DXVECTOR3 pSize;
	pSize.x = pPlayer->GetRadius() * 2.0f;
	pSize.z = pPlayer->GetRadius() * 2.0f;
	pSize.y = pPlayer->GetHeight() + pPlayer->GetRadius() * 2.0f;

	// 最小値と最大値を求める
	D3DXVECTOR3 playerMin = pPlayer->GetColliderPos() - pSize * 0.5f;
	D3DXVECTOR3 playerMax = pPlayer->GetColliderPos() + pSize * 0.5f;

	//=========================================================================
	// 交差チェック
	//=========================================================================
	return IsHitOBBvsAABB(obb, playerMin, playerMax);
}