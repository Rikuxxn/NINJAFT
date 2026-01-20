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
#include "shadowS.h"
#include "game.h"
#include "manager.h"
#include "generateMap.h"
#include "motion.h"
#include "meshfield.h"


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
	m_insightCount		= 0;							// 発見された回数
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

	// 敵の巡回ポイントの設定をするため、マップジェネレーターから位置をもらう
	auto& patrolPoints = CGenerateMap::GetInstance()->GetPatrolPoints();

	if (!patrolPoints.empty())
	{
		SetPatrolPoints(patrolPoints);
	}

	// 最初の巡回ポイントを決めておく
	ReturnToPatrol();

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CEnemy::Uninit(void)
{
	// 当たり判定の破棄
	ReleasePhysics();

	// モデルの破棄
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
	// 一番近い埋蔵金の場所を設定
	SetNearestTreasurePosition();

	// 向きの更新処理
	UpdateRotation(0.09f);

	// コライダーの位置更新(オフセットを設定)
	UpdateCollider(D3DXVECTOR3(0, DEFAULT_COLLIDER_OFFSET, 0));// 足元に合わせる
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
	D3DXVECTOR3 forward(-m_mtxWorld._31, m_mtxWorld._32, -m_mtxWorld._33);

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
// サブ敵のアクション判定
//=============================================================================
bool CEnemy::IsSubAction(EEnemyAction type)
{
	auto subs = CCharacterManager::GetInstance().GetCharacters<CEnemySub>();

	for (auto* sub : subs)
	{
		if (sub->GetRequestedAction() == type)
		{
			// 一体でもリクエストされていたら
			return true;
		}
	}

	return false;
}
//=============================================================================
// リーダー敵のアクション判定
//=============================================================================
bool CEnemy::IsLeaderAction(EEnemyAction type)
{
	// リーダー敵の取得
	CEnemyLeader* pEnemyLeader = CCharacterManager::GetInstance().GetCharacter<CEnemyLeader>();

	if (pEnemyLeader)
	{
		// 指定した行動を要求されていたら
		if (pEnemyLeader->GetRequestedAction() == type)
		{
			return true;
		}
	}

	return false;
}
//=============================================================================
// 一番近い埋蔵金ポイントの設定処理
//=============================================================================
void CEnemy::SetNearestTreasurePosition(void)
{
	auto& list = CGenerateMap::GetInstance()->GetTreasurePositions();

	// リストが空になったら
	if (list.empty())
	{
		return;
	}

	float minDist = FLT_MAX;
	int closestIndex = 0;

	for (size_t nCnt = 0; nCnt < list.size(); ++nCnt)
	{
		D3DXVECTOR3 dis = list[nCnt] - GetPos();

		// 一番近い埋蔵金ポイントに設定する
		float dist = D3DXVec3Length(&dis);
		if (dist < minDist)
		{
			minDist = dist;
			closestIndex = (int)nCnt;
		}
	}

	m_nearestTreasurePosition = list[closestIndex];
}
//=============================================================================
// メッシュの接地判定
//=============================================================================
bool CEnemy::OnGroundMesh(const CMeshField* field, float footOffset)
{
	if (!field)
	{
		return false;
	}

	btTransform trans;
	btRigidBody* pRigidBody = GetRigidBody();
	pRigidBody->getMotionState()->getWorldTransform(trans);
	btVector3 btPos = trans.getOrigin();

	const float r = 15.0f;

	float h0 = field->GetHeight(btPos.getX(), btPos.getZ());
	float h1 = field->GetHeight(btPos.getX() + r, btPos.getZ());
	float h2 = field->GetHeight(btPos.getX() - r, btPos.getZ());

	float groundY = std::max(h0, std::max(h1, h2));
	float footY = groundY + footOffset;

	return (btPos.getY() <= footY + 1.0f);
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
	m_bOnGround			= false;	// 接地しているか
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
	// 敵の初期化処理
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
		m_pWeaponCollider = std::make_unique<CWeaponCollider>();

#ifdef _DEBUG
		// 武器コライダーモデルの生成
		m_pTipModel = CObjectX::Create("data/MODELS/weapon_collider.x", m_pWeaponCollider->GetCurrentTipPos(), INIT_VEC3, D3DXVECTOR3(1.0f, 1.0f, 1.0f));
		m_pBaseModel = CObjectX::Create("data/MODELS/weapon_collider.x", m_pWeaponCollider->GetCurrentBasePos(), INIT_VEC3, D3DXVECTOR3(1.0f, 1.0f, 1.0f));
#endif
	}

	// カプセルコライダーの設定
	CreatePhysics(CAPSULE_RADIUS, CAPSULE_HEIGHT, 5.0f);

	// ステンシルシャドウの生成
	m_pShadowS = CShadowS::Create("data/MODELS/stencilshadow.x", D3DXVECTOR3(1.0f, 1.0f, 1.0f));
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
	// モーションの破棄
	if (m_pMotion != nullptr)
	{
		delete m_pMotion;
		m_pMotion = nullptr;
	}

	// 敵の終了処理
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
	CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

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

	// メッシュフィールドの取得
	CMeshField* pMeshField = CGenerateMap::GetInstance()->GetMeshField();

	if (!pMeshField)
	{
		return;
	}

	// 接地判定
	m_bOnGround = OnGroundMesh(pMeshField, COLLIDER_OFFSET);

	// Bullet現在位置取得
	btTransform trans;
	btRigidBody* pRigidBody = GetRigidBody();
	pRigidBody->getMotionState()->getWorldTransform(trans);
	btVector3 btPos = trans.getOrigin();

	D3DXVECTOR3 RigidPos;
	btVector3 vel = pRigidBody->getLinearVelocity();

	if (m_bOnGround)	// 接地中
	{
		// 見た目位置を地形に吸着
		float groundY = pMeshField->GetHeight(btPos.getX(), btPos.getZ());
		float targetY = groundY + COLLIDER_OFFSET;

		float dy = targetY - btPos.getY();

		// バネ的に地面に吸着（上にも下にも動ける）
		vel.setY(dy * GRAVITY_RATE);

		pRigidBody->setLinearVelocity(vel);

		// 見た目
		SetPos(D3DXVECTOR3(btPos.getX(), groundY, btPos.getZ()));
	}
	else// 空中
	{
		// Bullet主導で見た目を更新
		RigidPos.x = btPos.getX();
		RigidPos.y = btPos.getY() - COLLIDER_OFFSET;
		RigidPos.z = btPos.getZ();

		SetPos(RigidPos);
	}
}


//=============================================================================
// サブ敵のコンストラクタ
//=============================================================================
CEnemySub::CEnemySub()
{
	// 学習AIの生成
	SetAI(std::make_unique<CEnemyAI_Sub>());

	// 値のクリア
	m_pMotion	= nullptr;	// モーションへのポインタ
	m_bOnGround = false;	// 接地判定
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
	CreatePhysics(CAPSULE_RADIUS, CAPSULE_HEIGHT, 0.1f);

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

	// 敵の終了処理
	CEnemy::Uninit();
}
//=============================================================================
// サブ敵の更新処理
//=============================================================================
void CEnemySub::Update(void)
{
	CEnemy::Update();  // 共通処理

	// プレイヤーの取得
	CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

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

	// メッシュフィールドの取得
	CMeshField* pMeshField = CGenerateMap::GetInstance()->GetMeshField();

	if (!pMeshField)
	{
		return;
	}

	// 接地判定
	m_bOnGround = OnGroundMesh(pMeshField, COLLIDER_OFFSET);

	// Bullet現在位置取得
	btTransform trans;
	btRigidBody* pRigidBody = GetRigidBody();
	pRigidBody->getMotionState()->getWorldTransform(trans);
	btVector3 btPos = trans.getOrigin();

	D3DXVECTOR3 RigidPos;
	btVector3 vel = pRigidBody->getLinearVelocity();

	if (m_bOnGround)	// 接地中
	{
		// 見た目位置を地形に吸着
		float groundY = pMeshField->GetHeight(btPos.getX(), btPos.getZ());
		float targetY = groundY + COLLIDER_OFFSET;

		float dy = targetY - btPos.getY();

		// バネ的に地面に吸着（上にも下にも動ける）
		vel.setY(dy * GRAVITY_RATE);

		pRigidBody->setLinearVelocity(vel);

		// 見た目
		SetPos(D3DXVECTOR3(btPos.getX(), groundY, btPos.getZ()));
	}
	else// 空中
	{
		// Bullet主導で見た目を更新
		RigidPos.x = btPos.getX();
		RigidPos.y = btPos.getY() - COLLIDER_OFFSET;
		RigidPos.z = btPos.getZ();

		SetPos(RigidPos);
	}
}
