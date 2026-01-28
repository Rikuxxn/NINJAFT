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
#include "tutorial.h"
#include "meshfield.h"
#include "generateMap.h"


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
	m_pTargetTreasure	= nullptr;						// 埋蔵金ブロックへのポインタ
	m_bIsMoving			= false;						// 移動入力フラグ
	m_bOnGround			= false;						// 接地フラグ
	m_pDebug3D			= nullptr;						// 3Dデバッグ表示へのポインタ
	m_isInGrass			= false;						// 草の範囲内か
	m_isInTorch			= false;						// 灯籠の範囲内か
	m_isStealth			= false;						// ステルス状態か
	m_canControl		= false;						// 操作フラグ
	m_smokeTimer		= 30;							// 煙生成時間
	m_smokeActive		= true;							// 煙フラグ
	m_isGameStartSmoke	= true;							// ゲーム開始フラグ
	m_isDead			= false;						// 死亡したか
	m_bDamagePhysics	= false;						// ダメージ時に重力を使うか
	m_HeartbeatCnt		= 0;							// 心音カウンター
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

	// nullptrだったら
	if (pPlayer == nullptr)
	{
		return nullptr;
	}

	pPlayer->SetPos(pos);
	pPlayer->SetRot(D3DXToRadian(rot));
	pPlayer->SetSize(D3DXVECTOR3(1.2f, 1.2f, 1.2f));

	// 初期化失敗時
	if (FAILED(pPlayer->Init()))
	{
		return nullptr;
	}

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
	m_pShadowS = CShadowS::Create("data/MODELS/stencilshadow.x", D3DXVECTOR3(1.0f, 1.0f, 1.0f));
	m_pShadowS->SetStencilRef(1);// 個別のステンシルバッファの参照値を設定

	// インスタンスのポインタを渡す
	m_stateMachine.Start(this);

	// 初期状態のステートをセット
	m_stateMachine.ChangeState<CPlayer_StartState>();

	// HPの設定
	SetHp(10.0f);

	//// ゲージを生成
	//SetGuages({ 100.0f, 100.0f, 0.0f }, { 0.0f,1.0f,0.0f,1.0f }, { 1.0f,0.0f,0.0f,1.0f }, 420.0f, 20.0f);

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CPlayer::Uninit(void)
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

	// モーションの破棄
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

	// ゲームパッドの取得
	CInputJoypad* pJoypad = CManager::GetInputJoypad();

	// メッシュフィールドの取得
	CMeshField* pMeshField = CGenerateMap::GetInstance()->GetMeshField();

	if (pMeshField)
	{
		// 接地判定
		m_bOnGround = OnGroundMesh(pMeshField, 35.0f);
	}

	// ステートマシン更新
	m_stateMachine.Update();

	// 入力判定の取得
	InputData input = GatherInput();

#ifdef _DEBUG
	// キーボードの取得
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
	
	// プレイヤーHPが少ない(半分以下)
	IsHpFew hpFew;
	
	// プレイヤーHPがとても少ない
	IsHpVeryFew hpVeryFew;

	if (hpFew.IsSatisfiedBy(*this) && !m_isDead)
	{
		m_HeartbeatCnt++;

		if (hpVeryFew.IsSatisfiedBy(*this))
		{
			if (m_HeartbeatCnt >= HEARTBEART_INTERVAL_2)
			{
				m_HeartbeatCnt = 0;

				// 振動
				pJoypad->SetVibration(5000, 5000, 15);
			}
		}
		else
		{
			if (m_HeartbeatCnt >= HEARTBEART_INTERVAL_1)
			{
				m_HeartbeatCnt = 0;

				// 振動
				pJoypad->SetVibration(5000, 5000, 20);
			}
		}
	}

	// 特定のブロックに当たったか判定するため、ブロックマネージャーを取得する
	CBlockManager* pBlockManager = CGame::GetBlockManager();

	// 特定のブロックに当たっているか判定する
	bool playerInGrass = pBlockManager->IsPlayerInGrass();

	// ゲーム開始時
	if (m_isGameStartSmoke)
	{
		m_smokeActive = true;
		m_smokeTimer = 10;
		m_isGameStartSmoke = false;
	}

	if (m_smokeActive && !m_isGameStartSmoke)
	{
		for (int nCnt = 0; nCnt < EFFECT_CREATE_NUM; nCnt++)
		{
			D3DXVECTOR3 pos = GetPos();

			// 高さを徐々に上げる
			pos.y += static_cast<float>(nCnt) * HEIGHT_STEP;

			CParticle::Create<CSmokeParticle>(
				INIT_VEC3,
				pos,
				INIT_XCOL_WHITE,
				120,
				8
				);
		}

		if (--m_smokeTimer <= 0)
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

	// Bullet現在位置取得
	btTransform trans;
	btRigidBody* pRigidBody = GetRigidBody();
	pRigidBody->getMotionState()->getWorldTransform(trans);
	btVector3 btPos = trans.getOrigin();

	D3DXVECTOR3 RigidPos;
	btVector3 vel = pRigidBody->getLinearVelocity();

	if (pMeshField && m_bOnGround && !m_bDamagePhysics)	// 接地中
	{
		// 見た目位置を地形に吸着
		float groundY = pMeshField->GetHeight(btPos.getX(), btPos.getZ());
		float targetY = groundY + COLLIDER_OFFSET;

		float dy = targetY - btPos.getY();

		// バネ的に地面に吸着（上にも下にも動ける）
		vel.setY(dy * 5.0f);

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

	// 位置の取得
	D3DXVECTOR3 pos = GetPos();

	if (pos.y < RESPAWN_HEIGHT)
	{
		// リスポーン処理
		Respawn(D3DXVECTOR3(0.0f, 30.0f, -300.0f));
	}

	if (m_pShadowS != nullptr)
	{
		// ステンシルシャドウの位置設定
		m_pShadowS->SetPosition(GetPos());
	}

	// 時間の取得
	CTime* pTime = CGame::GetTime();

	bool isNight = false;

	if (pTime)
	{
		// 夜か判定
		isNight = pTime->IsNight();
	}

	// 特定のブロックに当たっているか判定する
	bool playerInTorch = pBlockManager->IsPlayerInTorch() && isNight;

	D3DXVECTOR4 outlineColor = VEC4_BLACK; // 通常は黒
	float modelAlpha = 1.0f;// モデルのアルファ値

	if (playerInTorch)// 灯籠に近づく
	{
		if (!m_isStealth && m_bIsMoving)
		{
			outlineColor = VEC4_RED; // 赤色
			modelAlpha = 1.0f;
		}
		else
		{
			outlineColor = VEC4_YELLOW; // 黄色
			modelAlpha = 1.0f;
		}
	}
	else if (playerInGrass)// 草に入る
	{
		if (!m_isStealth && m_bIsMoving)
		{
			outlineColor = VEC4_RED; // 赤色
			modelAlpha = 1.0f;
		}
		else
		{
			outlineColor = D3DXVECTOR4(1.0f, 1.0f, 1.0f, 0.1f);	// 半透明
			modelAlpha = 0.1f;
		}
	}

	// 色を適用
	CModel** models = GetModels();
	int num = GetNumModels();

	for (int nCnt = 0; nCnt < num; nCnt++)
	{
		// アウトラインカラーの設定
		models[nCnt]->SetOutlineColor(outlineColor);

		// モデルカラーの設定
		models[nCnt]->SetCol(D3DXCOLOR(1.0f, 1.0f, 1.0f, modelAlpha));
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

	btRigidBody* pRigid = GetRigidBody();
	btCollisionShape* pShape = GetCollisionShape();

	// カプセルコライダーの描画
	if (pRigid && pShape)
	{
		btTransform transform;
		pRigid->getMotionState()->getWorldTransform(transform);

		m_pDebug3D->DrawCapsuleCollider((btCapsuleShape*)pShape, transform, D3DXCOLOR(1, 1, 1, 1));
	}

#endif

}
//=============================================================================
// リスポーン(直接設定)処理
//=============================================================================
void CPlayer::Respawn(D3DXVECTOR3 pos)
{
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
// メッシュの接地判定
//=============================================================================
bool CPlayer::OnGroundMesh(const CMeshField* field, float footOffset)
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
// プレイヤーの前方ベクトル取得
//=============================================================================
D3DXVECTOR3 CPlayer::GetForward(void)
{
	D3DXVECTOR3 forward(-m_mtxWorld._31, m_mtxWorld._32, -m_mtxWorld._33);

	// 正規化する
	D3DXVec3Normalize(&forward, &forward);

	return forward;
}
//=============================================================================
// 入力判定取得関数
//=============================================================================
CPlayer::InputData CPlayer::GatherInput(void)
{
	InputData input{};
	input.moveDir = D3DXVECTOR3(0, 0, 0);
	input.attack = false;
	input.stealth = false;
	m_isStealth = false;

	CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();	// キーボードの取得
	CInputJoypad* pJoypad = CManager::GetInputJoypad();			// ジョイパッドの取得
	XINPUT_STATE* pStick = pJoypad->GetStickAngle();		// スティックの取得
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
	if (m_pMotion->IsCurrentMotion(DAMAGE) || m_isDead)
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
}
