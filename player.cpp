//=============================================================================
//
// プレイヤー処理 [player.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "player.h"
#include "texture.h"
#include "particle.h"
#include "guage.h"
#include "manager.h"
#include "enemy.h"
#include "playerState.h"
#include "time.h"
#include "shadowS.h"

// 名前空間stdの使用
using namespace std;

//=============================================================================
// コンストラクタ
//=============================================================================
CPlayer::CPlayer()
{
	// 値のクリア
	memset(m_apModel, 0, sizeof(m_apModel));			// モデル(パーツ)へのポインタ
	m_mtxWorld			= {};							// ワールドマトリックス
	m_nNumModel			= 0;							// モデル(パーツ)の総数
	m_pShadowS			= nullptr;						// ステンシルシャドウへのポインタ
	m_pMotion			= nullptr;						// モーションへのポインタ
	m_bIsMoving			= false;						// 移動入力フラグ
	m_bOnGround			= false;						// 接地フラグ
	m_pDebug3D			= nullptr;						// 3Dデバッグ表示へのポインタ
	m_isInGrass			= false;						// 草の範囲内か
	m_isInTorch			= false;						// 灯籠の範囲内か
	m_isStealth			= false;						// ステルス状態か
	m_prevIn			= false;						// 直前に入ったか
	m_canControl		= false;						// 操作フラグ
	m_smokeTimer		= 30;							// 煙生成時間
	m_smokeActive		= true;							// 煙フラグ
	m_isDead			= false;						// 死亡したか
}
//=============================================================================
// デストラクタ
//=============================================================================
CPlayer::~CPlayer()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CPlayer* CPlayer::Create(D3DXVECTOR3 pos, D3DXVECTOR3 rot)
{
	CPlayer* pPlayer = new CPlayer;

	pPlayer->SetPos(pos);
	pPlayer->SetRot(D3DXToRadian(rot));
	pPlayer->SetSize(D3DXVECTOR3(1.2f, 1.2f, 1.2f));

	// 初期化処理
	pPlayer->Init();

	return pPlayer;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CPlayer::Init(void)
{
	CModel* pModels[MAX_PARTS];
	int nNumModels = 0;

	// パーツの読み込み
	m_pMotion = CMotion::Load("data/motion.txt", pModels, nNumModels, MAX);

	for (int nCnt = 0; nCnt < nNumModels && nCnt < MAX_PARTS; nCnt++)
	{
		m_apModel[nCnt] = pModels[nCnt];

		// オフセット考慮
		m_apModel[nCnt]->SetOffsetPos(m_apModel[nCnt]->GetPos());
		m_apModel[nCnt]->SetOffsetRot(m_apModel[nCnt]->GetRot());
	}

	// パーツ数を代入
	m_nNumModel = nNumModels;

	// 目標の向きを設定
	SetRotDest(D3DXVECTOR3(0.0f, D3DXToRadian(180.0f), 0.0f));

	// カプセルコライダーの設定
	CreatePhysics(CAPSULE_RADIUS, CAPSULE_HEIGHT, 2.0f);

	// ステンシルシャドウの生成
	m_pShadowS = CShadowS::Create("data/MODELS/stencilshadow.x",GetPos());
	m_pShadowS->SetStencilRef(1);// 個別のステンシルバッファの参照値を設定

	// インスタンスのポインタを渡す
	m_stateMachine.Start(this);

	// 初期状態のステートをセット
	m_stateMachine.ChangeState<CPlayer_StandState>();

	// HPの設定
	SetHp(10.0f);

	// ゲージを生成
	SetGuages({ 100.0f, 100.0f, 0.0f }, { 0.0f,1.0f,0.0f,1.0f }, { 1.0f,0.0f,0.0f,1.0f }, 420.0f, 20.0f);

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CPlayer::Uninit(void)
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

	if (m_pMotion != nullptr)
	{
		delete m_pMotion;
		m_pMotion = nullptr;
	}

	// オブジェクトの破棄(自分自身)
	this->Release();
}
//=============================================================================
// 更新処理
//=============================================================================
void CPlayer::Update(void)
{
	// カメラの取得
	CCamera* pCamera = CManager::GetCamera();

	// カメラの角度の取得
	D3DXVECTOR3 CamRot = pCamera->GetRot();

	// 接地判定
	m_bOnGround = OnGround(CManager::GetPhysicsWorld(), GetRigidBody(), 55.0f);

	// ステートマシン更新
	m_stateMachine.Update();

	// 入力判定の取得
	InputData input = GatherInput();

#ifdef _DEBUG
	CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();

	if (pKeyboard->GetTrigger(DIK_1))
	{
		// ダメージ処理
		Damage(1.0f);
	}
	else if (pKeyboard->GetTrigger(DIK_2))
	{
		// 回復処理
		Heal(1.0f);
	}

#endif
	
	if (m_smokeActive)
	{
		D3DXVECTOR3 offPos = GetPos();
		offPos.y += 40.0f;

		for (int n = 0; n < 10; n++)
		{
			// 煙パーティクルの生成
			CParticle::Create<CSmokeParticle>(INIT_VEC3, offPos, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 120, 1);
		}

		m_smokeTimer--;

		if (m_smokeTimer <= 0)
		{
			m_smokeActive = false;
		}
	}

	// 向きの更新処理
	UpdateRotation(0.09f);

	// 移動入力があればプレイヤー向きを入力方向に
	if (input.moveDir.x != 0.0f || input.moveDir.z != 0.0f)
	{
		// Y成分だけを使いたいので目標の向きを取得
		D3DXVECTOR3 rotDest = GetRotDest();

		// Yを入力方向に向ける
		rotDest.y = atan2f(-input.moveDir.x, -input.moveDir.z);

		// 目標の向きに設定する
		SetRotDest(rotDest);
	}

	// コライダーの位置更新(オフセットを設定)
	UpdateCollider(D3DXVECTOR3(0, 35.0f, 0));// 足元に合わせる

	if (GetPos().y < -280.0f)
	{
		// リスポーン処理
		Respawn(D3DXVECTOR3(0.0f, 30.0f, -300.0f));
	}

	if (m_pShadowS != nullptr)
	{
		// ステンシルシャドウの位置設定
		m_pShadowS->SetPosition(GetPos());
	}

	// 時間の割合を取得
	float progress = CGame::GetTime()->GetProgress(); // 0.0～0.1
	bool isNight = (progress > 0.30f && progress <= 0.90f);

	// 特定のブロックに当たったか判定するため、ブロックマネージャーを取得する
	CBlockManager* pBlockManager = CGame::GetBlockManager();

	// 特定のブロックに当たっているか判定する
	bool playerInGrass = pBlockManager->IsPlayerInGrass();
	bool playerInTorch = pBlockManager->IsPlayerInTorch() && isNight;

	D3DXVECTOR4 outlineColor = D3DXVECTOR4(0, 0, 0, 1); // 通常は黒

	if (playerInTorch)// 灯籠に近づく
	{
		if (!m_isStealth && m_bIsMoving)
		{
			outlineColor = D3DXVECTOR4(1, 0, 0, 1); // 赤色
		}
		else
		{
			outlineColor = D3DXVECTOR4(1, 1, 0, 1); // 黄色
		}
	}
	else if (playerInGrass)// 草に入る
	{
		if (!m_isStealth && m_bIsMoving)
		{
			outlineColor = D3DXVECTOR4(1, 0, 0, 1); // 赤色
		}
		else
		{
			outlineColor = D3DXVECTOR4(1, 1, 1, 1);	// 白
		}
	}

	// 色を適用
	CModel** models = GetModels();
	for (int i = 0; i < GetNumModels(); i++)
	{
		models[i]->SetOutlineColor(outlineColor);
	}

	// モーションの更新処理
	m_pMotion->Update(m_apModel, m_nNumModel);
}
//=============================================================================
// 描画処理
//=============================================================================
void CPlayer::Draw(void)
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
// リスポーン(直接設定)処理
//=============================================================================
void CPlayer::Respawn(D3DXVECTOR3 pos)
{
	if (CManager::GetMode() != MODE_GAME)
	{
		return;
	}

	D3DXVECTOR3 respawnPos = pos; // 任意の位置

	GetPos() = respawnPos;

	btRigidBody* pRigid = GetRigidBody();

	if (pRigid)
	{
		pRigid->setLinearVelocity(btVector3(0, 0, 0));
		pRigid->setAngularVelocity(btVector3(0, 0, 0));

		// ワールド座標更新
		btTransform trans;
		trans.setIdentity();
		trans.setOrigin(btVector3(respawnPos.x, respawnPos.y, respawnPos.z));

		pRigid->setWorldTransform(trans);

		if (pRigid->getMotionState())
		{
			pRigid->getMotionState()->setWorldTransform(trans);
		}
	}
}
//=============================================================================
// レイによる接触判定
//=============================================================================
bool CPlayer::OnGround(btDiscreteDynamicsWorld* world, btRigidBody* playerBody, float rayLength)
{
	btTransform trans;
	playerBody->getMotionState()->getWorldTransform(trans);

	btVector3 start = trans.getOrigin();
	btVector3 end = start - btVector3(0, rayLength, 0);

	struct RayResultCallback : public btCollisionWorld::ClosestRayResultCallback
	{
		RayResultCallback(const btVector3& from, const btVector3& to)
			: btCollisionWorld::ClosestRayResultCallback(from, to) {}
	};

	RayResultCallback rayCallback(start, end);
	world->rayTest(start, end, rayCallback);

	if (rayCallback.hasHit())
	{// 接触
		// ヒットした面の法線
		btVector3 normal = rayCallback.m_hitNormalWorld.normalized();

		// 上方向ベクトル
		btVector3 up(0, 1, 0);

		// 法線と上方向の内積（= cosθ）
		float slopeDot = normal.dot(up);

		// cosθ が小さいほど急斜面
		// 50度までを地面として扱う
		float slopeLimit = cosf(D3DXToRadian(50.0f));

		if (slopeDot >= slopeLimit)
		{
			return true; // 接地
		}
		else
		{
			return false; // 急斜面なので接地扱いしない
		}
	}

	return false;
}
//=============================================================================
// プレイヤーの前方ベクトル取得
//=============================================================================
D3DXVECTOR3 CPlayer::GetForward(void)
{
	// プレイヤーの回転角度（Y軸）から前方ベクトルを計算
	float yaw = GetRot().y;

	D3DXVECTOR3 forward(-sinf(yaw), 0.0f, -cosf(yaw));

	// 正規化する
	D3DXVec3Normalize(&forward, &forward);

	return forward;
}
//=============================================================================
// プレイヤーの前方レイ判定処理
//=============================================================================
CBlock* CPlayer::FindFrontBlockByRaycast(float rayLength)
{
	btDiscreteDynamicsWorld* world = CManager::GetPhysicsWorld();

	if (!world)
	{
		return nullptr;
	}

	D3DXVECTOR3 from = GetPos() + D3DXVECTOR3(0.0f,20.0f,0.0f);
	D3DXVECTOR3 to = from + GetForward() * rayLength;

	btVector3 btFrom(from.x, from.y, from.z);
	btVector3 btTo(to.x, to.y, to.z);

	struct RayResultCallback : public btCollisionWorld::ClosestRayResultCallback
	{
		RayResultCallback(const btVector3& from, const btVector3& to)
			: btCollisionWorld::ClosestRayResultCallback(from, to) {}

		virtual btScalar addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace)
		{
			// ブロック以外を無視する場合はここでフィルターしてもいい
			return ClosestRayResultCallback::addSingleResult(rayResult, normalInWorldSpace);
		}
	};

	RayResultCallback rayCallback(btFrom, btTo);
	world->rayTest(btFrom, btTo, rayCallback);

	if (rayCallback.hasHit())
	{
		void* userPtr = rayCallback.m_collisionObject->getUserPointer();

		if (userPtr)
		{
			CBlock* pBlock = static_cast<CBlock*>(userPtr);

			if (pBlock->IsDynamicBlock())
			{
				return pBlock; // 動的なブロックが前方にあった
			}
		}
	}

	return nullptr;
}
//=============================================================================
// 入力判定取得関数
//=============================================================================
InputData CPlayer::GatherInput(void)
{
	InputData input{};
	input.moveDir = D3DXVECTOR3(0, 0, 0);
	input.attack = false;
	input.stealth = false;
	m_isStealth = false;

	CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();	// キーボードの取得
	CInputJoypad* pJoypad = CManager::GetInputJoypad();			// ジョイパッドの取得
	XINPUT_STATE* pStick = CInputJoypad::GetStickAngle();		// スティックの取得
	CCamera* pCamera = CManager::GetCamera();					// カメラの取得
	D3DXVECTOR3 CamRot = pCamera->GetRot();						// カメラ角度の取得

	if (this == nullptr || !m_canControl)
	{
		return input;
	}

	// ---------------------------
	// 忍び足入力
	// ---------------------------
	if (pKeyboard->GetPress(DIK_LCONTROL) || pJoypad->GetPressR2())
	{
		input.stealth = true;
	}

	m_isStealth = input.stealth || !m_bIsMoving;

	// ---------------------------
	// タメージ状態中は移動入力無効化
	// ---------------------------
	if (m_pMotion->IsCurrentMotion(DAMAGE))
	{
		return input;
	}

	// ---------------------------
	// ゲームパッド入力
	// ---------------------------
	if (pJoypad->GetStick() && pStick)
	{
		float stickX = pStick->Gamepad.sThumbLX;
		float stickY = pStick->Gamepad.sThumbLY;
		float magnitude = sqrtf(stickX * stickX + stickY * stickY);
		const float DEADZONE = 10922.0f;

		if (magnitude >= DEADZONE)
		{
			stickX /= magnitude;
			stickY /= magnitude;
			float normMag = std::min((magnitude - DEADZONE) / (32767.0f - DEADZONE), 1.0f);
			stickX *= normMag;
			stickY *= normMag;

			D3DXVECTOR3 dir;
			float yaw = CamRot.y;

			dir.x = -(stickX * cosf(yaw) + stickY * sinf(yaw));
			dir.z = stickX * sinf(-yaw) + stickY * cosf(yaw);
			dir.z = -dir.z;

			input.moveDir += D3DXVECTOR3(dir.x, 0, dir.z);
		}
	}

	// ---------------------------
	// キーボード入力
	// ---------------------------
	if (pKeyboard->GetPress(DIK_W))
	{
		input.moveDir += D3DXVECTOR3(-sinf(CamRot.y), 0, -cosf(CamRot.y));
	}
	if (pKeyboard->GetPress(DIK_S))
	{
		input.moveDir += D3DXVECTOR3(sinf(CamRot.y), 0, cosf(CamRot.y));
	}
	if (pKeyboard->GetPress(DIK_A))
	{
		input.moveDir += D3DXVECTOR3(cosf(CamRot.y), 0, -sinf(CamRot.y));
	}
	if (pKeyboard->GetPress(DIK_D))
	{
		input.moveDir += D3DXVECTOR3(-cosf(CamRot.y), 0, sinf(CamRot.y));
	}

	// 正規化
	if (input.moveDir.x != 0.0f || input.moveDir.z != 0.0f)
	{
		D3DXVec3Normalize(&input.moveDir, &input.moveDir);
	}

	return input;
}
//=============================================================================
// ダメージ処理
//=============================================================================
void CPlayer::Damage(float fDamage)
{
	if (!m_pMotion->IsCurrentMotion(DAMAGE))
	{
		// まず共通のHP処理
		CCharacter::Damage(fDamage);

		// ダメージステートへ
		m_stateMachine.ChangeState<CPlayer_DamageState>();
	}

	// 死亡時
	if (IsDead())
	{
		//// 死亡状態
		//m_stateMachine.ChangeState<CPlayer_DeadState>();
		return;
	}
}

