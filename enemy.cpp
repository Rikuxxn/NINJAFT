//=============================================================================
//
// 敵の処理 [enemy.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "enemy.h"
#include "guage.h"
#include "enemyleaderState.h"
#include "enemysubState.h"
#include "ui.h"

// 名前空間stdの使用
using namespace std;

//=============================================================================
// コンストラクタ
//=============================================================================
CEnemy::CEnemy()
{
	// 値のクリア
	memset(m_apModel, 0, sizeof(m_apModel));			// モデル(パーツ)へのポインタ
	m_mtxWorld			= {};							// ワールドマトリックス
	m_nNumModel			= 0;							// モデル(パーツ)の総数
	m_pDebug3D			= nullptr;						// 3Dデバッグ表示へのポインタ
	m_requestedAction	= ACTION_NONE;					// AIの行動リクエスト
	m_sightRange		= 255.0f;						// 視界距離
	m_sightAngle		= D3DXToRadian(110.0f);			// 視界範囲
	m_lastHeardSoundPos = INIT_VEC3;					// 最後に聞いた音の座標
	m_hasHeardSound		= false;						// 音を聞いたかどうか
	m_returnToPatrol	= false;						// 最寄りの巡回ポイントに戻るフラグ
	m_canControl		= false;						// 操作フラグ
	m_makeSoundCount	= 0;							// 音発生数
}
//=============================================================================
// デストラクタ
//=============================================================================
CEnemy::~CEnemy()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CEnemy::Init(void)
{
	// 最初の向き
	SetRot(D3DXVECTOR3(0.0f, -D3DX_PI, 0.0f));

	// 巡回ポイントの設定
	auto& patrolPoints = CGame::GetBlockManager()->GetPatrolPoints();
	SetPatrolPoints(patrolPoints);

	// 最初の巡回ポイントを決めておく
	ReturnToPatrol();

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CEnemy::Uninit(void)
{
	ReleasePhysics();

	for (int nCnt = 0; nCnt < MAX_PARTS; nCnt++)
	{
		if (m_apModel[nCnt] != nullptr)
		{
			m_apModel[nCnt]->Uninit();
			delete m_apModel[nCnt];
			m_apModel[nCnt] = nullptr;
		}
	}

	// オブジェクトの破棄(自分自身)
	this->Release();
}
//=============================================================================
// 更新処理
//=============================================================================
void CEnemy::Update(void)
{

#ifdef _DEBUG
	CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();

	if (pKeyboard->GetTrigger(DIK_3))
	{
		// ダメージ処理
		Damage(50.0f);
	}
	else if (pKeyboard->GetTrigger(DIK_4))
	{
		// 回復処理
		Heal(1.0);
	}

#endif

	// 向きの更新処理
	UpdateRotation(0.09f);

	// コライダーの位置更新(オフセットを設定)
	UpdateCollider(D3DXVECTOR3(0, 45.0f, 0));// 足元に合わせる
}
//=============================================================================
// 描画処理
//=============================================================================
void CEnemy::Draw(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// 計算用マトリックス
	D3DXMATRIX mtxRot, mtxTrans, mtxSize;

	// ワールドマトリックスの初期化
	D3DXMatrixIdentity(&m_mtxWorld);

	// サイズを反映
	D3DXMatrixScaling(&mtxSize, GetSize().x, GetSize().y, GetSize().z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxSize);

	// 向きを反映
	D3DXMatrixRotationYawPitchRoll(&mtxRot, GetRot().y, GetRot().x, GetRot().z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxRot);

	// 位置を反映
	D3DXMatrixTranslation(&mtxTrans, GetPos().x, GetPos().y, GetPos().z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxTrans);

	// ワールドマトリックスを設定
	pDevice->SetTransform(D3DTS_WORLD, &m_mtxWorld);

	for (int nCntMat = 0; nCntMat < m_nNumModel; nCntMat++)
	{
		// モデル(パーツ)の描画
		if (m_apModel[nCntMat])
		{
			m_apModel[nCntMat]->Draw();
		}
	}

#ifdef _DEBUG

	//btRigidBody* pRigid = GetRigidBody();
	//btCollisionShape* pShape = GetCollisionShape();

	//// カプセルコライダーの描画
	//if (pRigid && pShape)
	//{
	//	btTransform transform;
	//	pRigid->getMotionState()->getWorldTransform(transform);

	//	m_pDebug3D->DrawCapsuleCollider((btCapsuleShape*)pShape, transform, D3DXCOLOR(1, 1, 1, 1));
	//}

#endif

}
//=============================================================================
// 前方ベクトル取得
//=============================================================================
D3DXVECTOR3 CEnemy::GetForward(void)
{
	// 回転角度（Y軸）から前方ベクトルを計算
	float yaw = GetRot().y;

	D3DXVECTOR3 forward(-sinf(yaw), 0.0f, -cosf(yaw));

	// 正規化する
	D3DXVec3Normalize(&forward, &forward);

	return forward;
}
//=============================================================================
// 視界内判定
//=============================================================================
bool CEnemy::IsPlayerInSight(CPlayer* pPlayer)
{
	// 敵の正面ベクトルを取得
	D3DXVECTOR3 enemyFront = GetForward();

	// プレイヤーとの方向ベクトル
	D3DXVECTOR3 toPlayer = pPlayer->GetPos() - GetPos();

	// プレイヤー方向ベクトルを正規化
	D3DXVec3Normalize(&toPlayer, &toPlayer);

	// 敵の正面ベクトルも正規化
	D3DXVec3Normalize(&enemyFront, &enemyFront);

	// ベクトルの内積を計算
	float dotProduct = D3DXVec3Dot(&enemyFront, &toPlayer);

	// 内積から視野内か判定
	if (dotProduct > cosf(m_sightAngle * 0.5f)) // 視野角の半分で判定
	{
		// プレイヤーとの距離を計算
		float distanceSquared =
			(GetPos().x - pPlayer->GetPos().x) * (GetPos().x - pPlayer->GetPos().x) +
			(GetPos().y - pPlayer->GetPos().y) * (GetPos().y - pPlayer->GetPos().y) +
			(GetPos().z - pPlayer->GetPos().z) * (GetPos().z - pPlayer->GetPos().z);

		if (distanceSquared <= m_sightRange * m_sightRange)
		{
			return true; // プレイヤーは視界内
		}
	}

	return false; // 視界外
}


//=============================================================================
// リーダー敵のコンストラクタ
//=============================================================================
CEnemyLeader::CEnemyLeader()
{
	// 学習AIの生成
	SetAI(std::make_unique<CEnemyAI_Leader>());

	// 値のクリア
	m_pMotion			= nullptr;	// モーションへのポインタ
	m_pShadowS			= nullptr;	// ステンシルシャドウへのポインタ
	m_pTipModel			= nullptr;	// 武器コライダー用モデル
	m_pBaseModel		= nullptr;	// 武器コライダー用モデル
	m_pWeaponCollider	= nullptr;	// 武器の当たり判定へのポインタ
	m_Cooldown			= 0.0f;     // クールダウン残り時間
}
//=============================================================================
// リーダー敵のデストラクタ
//=============================================================================
CEnemyLeader::~CEnemyLeader()
{
	// なし
}
//=============================================================================
// リーダー敵の初期化処理
//=============================================================================
HRESULT CEnemyLeader::Init(void)
{
	CEnemy::Init();

	CModel* pModels[MAX_PARTS];
	int nNumModels = 0;

	// Leader 用モーション
	m_pMotion = CMotion::Load(
		"data/motion_enemy.txt",
		pModels,
		nNumModels,
		MAX
	);

	SetupModels(pModels, nNumModels);

	if (GetWeapon())
	{
		// 武器コライダーの生成
		m_pWeaponCollider = make_unique<CWeaponCollider>();

#ifdef _DEBUG
		// 武器コライダーモデルの生成
		m_pTipModel = CObjectX::Create("data/MODELS/weapon_collider.x", m_pWeaponCollider->GetCurrentTipPos(), INIT_VEC3, D3DXVECTOR3(1.0f, 1.0f, 1.0f));
		m_pBaseModel = CObjectX::Create("data/MODELS/weapon_collider.x", m_pWeaponCollider->GetCurrentBasePos(), INIT_VEC3, D3DXVECTOR3(1.0f, 1.0f, 1.0f));
#endif
	}

	// カプセルコライダーの設定
	CreatePhysics(CAPSULE_RADIUS, CAPSULE_HEIGHT, 5.0f);

	// ステンシルシャドウの生成
	m_pShadowS = CShadowS::Create("data/MODELS/stencilshadow.x", GetPos());
	m_pShadowS->SetStencilRef(2);// 個別のステンシルバッファの参照値を設定

	// インスタンスのポインタを渡す
	m_stateMachine.Start(this);

	// 初期状態のステートをセット
	m_stateMachine.ChangeState<CEnemyLeader_StandState>();

	return S_OK;
}
//=============================================================================
// リーダー敵の終了処理
//=============================================================================
void CEnemyLeader::Uninit(void)
{
	if (m_pMotion != nullptr)
	{
		delete m_pMotion;
		m_pMotion = nullptr;
	}

	CEnemy::Uninit();
}
//=============================================================================
// リーダー敵の更新処理
//=============================================================================
void CEnemyLeader::Update(void)
{
	CEnemy::Update();  // 共通処理

	if (m_pWeaponCollider != nullptr)
	{
		// 武器コライダーの更新
		m_pWeaponCollider->Update(GetWeapon(), 50.0f, 10.0f);
	}

#ifdef _DEBUG
	if (m_pWeaponCollider != nullptr)
	{
		// 武器コライダー用モデルの位置更新
		m_pTipModel->SetPos(m_pWeaponCollider->GetCurrentTipPos());
		m_pBaseModel->SetPos(m_pWeaponCollider->GetCurrentBasePos());
	}
#endif
	if (m_pShadowS != nullptr)
	{
		// ステンシルシャドウの位置設定
		m_pShadowS->SetPosition(GetPos());
	}

	if (m_Cooldown > 0.0f)
	{
		m_Cooldown--;// クールダウンを減らす

		if (m_Cooldown < 0.0f)
		{
			m_Cooldown = 0.0f;
		}
	}

	// プレイヤーの取得
	CPlayer* pPlayer = CGame::GetPlayer();

	// AIを更新（現在の行動のリクエスト）
	if (GetAI() && pPlayer)
	{
		GetAI()->Update(this, pPlayer);
	}

	// ステートマシン更新
	m_stateMachine.Update();

	int n = GetNumModels();

	// モーションの更新処理
	m_pMotion->Update(GetModels(), n);
}


//=============================================================================
// サブ敵のコンストラクタ
//=============================================================================
CEnemySub::CEnemySub()
{
	// 学習AIの生成
	SetAI(std::make_unique<CEnemyAI_Sub>());

	// 値のクリア
	m_pMotion = nullptr;	// モーションへのポインタ
}
//=============================================================================
// サブ敵のデストラクタ
//=============================================================================
CEnemySub::~CEnemySub()
{
	// なし
}
//=============================================================================
// サブ敵の初期化処理
//=============================================================================
HRESULT CEnemySub::Init(void)
{
	CEnemy::Init();

	CModel* pModels[MAX_PARTS];
	int nNumModels = 0;

	// sub 用モーション
	m_pMotion = CMotion::Load(
		"data/motion_subenemy.txt",
		pModels,
		nNumModels,
		MAX
	);

	SetupModels(pModels, nNumModels);

	// カプセルコライダーの設定
	CreatePhysics(0.0f, CAPSULE_HEIGHT, 0.1f);

	// インスタンスのポインタを渡す
	m_stateMachine.Start(this);

	// 初期状態のステートをセット
	m_stateMachine.ChangeState<CEnemySub_FollowState>();

	return S_OK;
}
//=============================================================================
// サブ敵の終了処理
//=============================================================================
void CEnemySub::Uninit(void)
{
	if (m_pMotion != nullptr)
	{
		delete m_pMotion;
		m_pMotion = nullptr;
	}

	CEnemy::Uninit();
}
//=============================================================================
// サブ敵の更新処理
//=============================================================================
void CEnemySub::Update(void)
{
	CEnemy::Update();  // 共通処理

	// プレイヤーの取得
	CPlayer* pPlayer = CGame::GetPlayer();

	// AIを更新（現在の行動のリクエスト）
	if (GetAI() && pPlayer)
	{
		GetAI()->Update(this, pPlayer);
	}

	// ステートマシン更新
	m_stateMachine.Update();

	int n = GetNumModels();

	// モーションの更新処理
	m_pMotion->Update(GetModels(), n);
}
