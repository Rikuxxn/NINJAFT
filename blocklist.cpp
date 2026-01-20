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
#include "tutorial.h"
#include "enemy.h"
#include "movie.h"
#include "easing.h"
#include "generateMap.h"


//=============================================================================
// 壁ブロックのコンストラクタ
//=============================================================================
CWallBlock::CWallBlock()
{
	// 値のクリア

}
//=============================================================================
// 壁ブロックのデストラクタ
//=============================================================================
CWallBlock::~CWallBlock()
{
	// なし
}
//=============================================================================
// 壁ブロックの更新処理
//=============================================================================
void CWallBlock::Update(void)
{
	// ブロックの更新処理
	CBlock::Update();

	if (CManager::GetMode() != CScene::MODE_RESULT)
	{
		return;
	}

	// 位置取得
	D3DXVECTOR3 pos = GetPos();

	// 奥(Z-方向)へ移動
	pos.z -= moveSpeed;

	// 一定の奥行きを超えたら(小さくなったら)手前に戻す
	if (pos.z <= resetDepth)
	{
		pos.z = bottomPosZ;
	}

	// 位置反映
	SetPos(pos);
}


//=============================================================================
// 草地面ブロックのコンストラクタ
//=============================================================================
CGrassFloorBlock::CGrassFloorBlock()
{
	// 値のクリア

}
//=============================================================================
// 草地面ブロックのデストラクタ
//=============================================================================
CGrassFloorBlock::~CGrassFloorBlock()
{
	// なし
}
//=============================================================================
// 草地面ブロックの更新処理
//=============================================================================
void CGrassFloorBlock::Update(void)
{
	// ブロックの更新処理
	CBlock::Update();

	if (CManager::GetMode() != CScene::MODE_RESULT)
	{
		return;
	}

	// 位置取得
	D3DXVECTOR3 pos = GetPos();

	// 奥(Z-方向)へ移動
	pos.z -= moveSpeed;

	// 一定の奥行きを超えたら(小さくなったら)手前に戻す
	if (pos.z <= resetDepth)
	{
		pos.z = bottomPosZ;
	}

	// 位置反映
	SetPos(pos);
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
	CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

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
	float progress = 0.0f;

	if (CManager::GetMode() == CScene::MODE_TUTORIAL)
	{
		// 時間の割合を取得
		progress = CTutorial::GetTime()->GetProgress(); // 0.0～0.1
	}
	else if (CManager::GetMode() == CScene::MODE_GAME)
	{
		// 時間の割合を取得
		progress = CGame::GetTime()->GetProgress(); // 0.0～0.1
	}

	// ======== 夜のフェード処理 ========
	float torchIntensity = 0.0f;

	// 夜になる手前（夕方）でフェードイン
	if (progress >= 0.25f && progress < 0.30f)
	{
		torchIntensity = (progress - 0.25f) / (0.30f - 0.25f);
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
// 桜の木ブロックのコンストラクタ
//=============================================================================
CBlossomTreeBlock::CBlossomTreeBlock()
{
	// 値のクリア

}
//=============================================================================
// 桜の木ブロックのデストラクタ
//=============================================================================
CBlossomTreeBlock::~CBlossomTreeBlock()
{
	// なし
}
//=============================================================================
// 桜の木ブロックの更新処理
//=============================================================================
void CBlossomTreeBlock::Update(void)
{
	// ブロックの更新処理
	CBlock::Update();

	if (CManager::GetMode() != CScene::MODE_RESULT)
	{
		return;
	}

	// 位置取得
	D3DXVECTOR3 pos = GetPos();

	// 奥(Z-方向)へ移動
	pos.z -= moveSpeed;

	// 一定の奥行きを超えたら(小さくなったら)手前に戻す
	if (pos.z <= resetDepth)
	{
		pos.z = bottomPosZ;
	}

	// 位置反映
	SetPos(pos);
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
	m_effectTimer			= 0;		// エフェクト生成タイマー
	m_guageRate				= 0.0f;		// ゲージの最大量
	m_guageDecreaseSpeed	= 0.0f;		// ゲージの減る量
	m_bUiActive				= false;	// UIが表示されているか
	m_isFinished			= false;	// 取得し終えたか
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

	// リセットしておく
	m_getCount = 0;

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

	// 音の取得
	CSound* pSound = CManager::GetSound();

	// プレイヤーの取得
	CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

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
		if (!isDamage && !isMoving && pPlayer->GetControlFlag())
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
		// アイテム取得SEの再生
		if (pSound)
		{
			pSound->Play(CSound::SOUND_LABEL_ITEMGET);
		}

		if (!m_isFinished)
		{
			m_isFinished = true;

			// 取得カウントを増やす
			m_getCount++;
		}

		// リーダー敵の取得
		CEnemyLeader* pEnemyLeader = CCharacterManager::GetInstance().GetCharacter<CEnemyLeader>();

		if (pEnemyLeader)
		{
			// 埋蔵金が取られたことを通知して、リストから位置を削除する
			CGenerateMap::GetInstance()->OnTreasureCollected(GetPos());
		}

		// ゲージも削除する
		m_pGuage->Uninit();
		m_pFrame->Uninit();

		// 削除予約
		Kill();
		return;
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
	m_baseRotY	= 0.0f;		// 基準の角度
	m_rotY		= 0.0f;		// Y角度
	m_prevOpen	= false;	// 直前に開いたか
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

	// 一回だけ通す
	bool n = m_isOpen;

	if (n && !m_prevOpen)
	{
		// 音の取得
		CSound* pSound = CManager::GetSound();

		// 開門SE
		if (pSound)
		{
			pSound->Play(CSound::SOUND_LABEL_GATE_OPEN);
		}
	}

	m_prevOpen = n;

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
	m_isIn = false;
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

	// プレイヤーの取得
	CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

	if (!pPlayer)
	{
		return;
	}

	// 脱出可能だったら
	if (AvailableExit())
	{
		CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();
		CInputJoypad* pJoypad = CManager::GetInputJoypad();

		// 対象との距離を求めるためにプレイヤーの位置を取得
		D3DXVECTOR3 playerPos = pPlayer->GetPos();

		// 対象との距離を求める
		D3DXVECTOR3 disPos = playerPos - GetPos();
		float distance = D3DXVec3Length(&disPos);

		// 範囲内だったら
		if (distance < TRIGGER_DISTACE)
		{
			// 範囲内フラグtrue
			m_isIn = true;

			// 任意のボタンを押したら
			if (pKeyboard->GetTrigger(DIK_F) || pJoypad->GetTrigger(CInputJoypad::JOYKEY_A))
			{
				m_isEscape = true;
			}
		}
		else
		{
			m_isIn = false;
		}
	}
}
//=============================================================================
// 接触判定処理
//=============================================================================
bool CExitBlock::IsHitPlayer(CPlayer* pPlayer)
{
	//=====================================================================
	// ブロック側（OBB）
	//=====================================================================
	OBB obb;
	obb.center = GetPos();

	D3DXMATRIX world = GetWorldMatrix();
	obb.axis[0] = D3DXVECTOR3(world._11, world._12, world._13);
	obb.axis[1] = D3DXVECTOR3(world._21, world._22, world._23);
	obb.axis[2] = D3DXVECTOR3(world._31, world._32, world._33);

	// 軸は念のため正規化
	for (int nCnt = 0; nCnt < 3; nCnt++)
	{
		D3DXVec3Normalize(&obb.axis[nCnt], &obb.axis[nCnt]);
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


//=============================================================================
// 門ブロックのコンストラクタ
//=============================================================================
CGateBlock::CGateBlock()
{
	// 値のクリア
	m_baseRotY		= 0.0f;			// 基準の角度
	m_movieTime		= DELAY_TIME;	// 遅延時間
	m_prevStep		= 0;			// 直前の段階数
	m_bClosing		= false;		// 閉じているか
	m_closeTimer	= 0.0f;			// タイマー
	m_fromX			= 0.0f;			// 移動開始位置
	m_toX			= 0.0f;			// 移動到達位置
}
//=============================================================================
// 門ブロックのデストラクタ
//=============================================================================
CGateBlock::~CGateBlock()
{
	// なし
}
//=============================================================================
// 門ブロックの更新処理
//=============================================================================
void CGateBlock::Update(void)
{
	// ブロックの更新処理
	CBlock::Update();

	// ゲームシーンだったら
	if (CManager::GetMode() == CScene::MODE_GAME)
	{
		GameGateUpdate();
	}
	// ムービーシーンだったら
	else if (CManager::GetMode() == CScene::MODE_MOVIE)
	{
		MovieGateUpdate();
	}
}
//=============================================================================
// ゲームシーン門ブロックの閉じる処理
//=============================================================================
void CGateBlock::GameGateUpdate(void)
{
	// 時間の割合を取得
	float progress = CGame::GetTime()->GetProgress(); // 0.0～1.0

	// 時間の割合に応じて段階で門を閉じる
	int step = 0;
	bool bShaking = false;

	if (progress >= 0.99f)
	{
		step = MAX_STEP; // 閉じ切る
	}
	else
	{
		step =(int)(progress * MAX_STEP);
		step = std::clamp(step, 0, MAX_STEP - 1);
	}

	// =================================
	// 予兆揺れ判定
	// =================================
	float shakeX = 0.0f;

	float nextBorder = (float)(step + 1) / MAX_STEP;

	if (!m_bClosing && progress < 0.99f &&
		progress >= nextBorder - PRE_SHAKE_RANGE)
	{
		bShaking = true;

		// 予兆揺れ
		shakeX = sinf(progress * 500.0f * SHAKE_SPEED) * SHAKE_POWER;

		// 少し下げた位置に生成
		D3DXVECTOR3 spawnBase = GetPos();
		spawnBase.y -= 60.0f;

		// 中心
		D3DXVECTOR3 spawnCenter = spawnBase;

		// 中心から左右
		D3DXVECTOR3 spawnLeft = spawnBase;
		spawnLeft.x -= SIDE_OFFSET;

		D3DXVECTOR3 spawnRight = spawnBase;
		spawnRight.x += SIDE_OFFSET;

		// 埃パーティクル生成
		CParticle::Create<CDustParticle>(INIT_VEC3, spawnCenter,
			D3DXCOLOR(0.8f, 0.8f, 0.8f, 0.3f), 90, 1);

		CParticle::Create<CDustParticle>(INIT_VEC3, spawnLeft,
			D3DXCOLOR(0.8f, 0.8f, 0.8f, 0.3f), 90, 1);

		CParticle::Create<CDustParticle>(INIT_VEC3, spawnRight,
			D3DXCOLOR(0.8f, 0.8f, 0.8f, 0.3f), 90, 1);
	}

	// =================================
	// 閉じ開始トリガー
	// =================================
	if (step != m_prevStep)
	{
		m_bClosing = true;
		m_closeTimer = 0.0f;

		m_fromX = m_prevStep * MOVE_UNIT;
		m_toX = step * MOVE_UNIT;
	}

	// =================================
	// 閉じアニメーション（イージング）
	// =================================
	float moveX = step * MOVE_UNIT;

	if (m_bClosing)
	{
		float t = m_closeTimer / CLOSE_DURATION;
		t = std::clamp(t, 0.0f, 1.0f);

		moveX = CEasing::Ease(m_fromX, m_toX, t, CEasing::EaseOutCubic);

		m_closeTimer++;

		if (t >= 1.0f)
		{
			m_bClosing = false;
		}
	}

	m_prevStep = step;

	// ジョイパッドの取得
	CInputJoypad* pJoypad = CManager::GetInputJoypad();

	// =================================
	// 振動制御
	// =================================
	if (bShaking && pJoypad)
	{
		// 振動させる
		pJoypad->SetVibration(20000, 20000);
	}
	else
	{
		// 振動停止
		pJoypad->StopVibration();
	}

	// 位置を取得して反映する
	D3DXVECTOR3 pos = GetPos();

	// 揺れを反映した位置にする
	if (m_baseRotY == 0.0f)
	{
		pos.x = m_startPosX + moveX + shakeX;
	}
	else if (m_baseRotY == -180.0f)
	{
		pos.x = m_startPosX - moveX + shakeX;
	}

	// 位置を設定する
	SetPos(pos);
}
//=============================================================================
// ムービーシーン門ブロックの更新処理
//=============================================================================
void CGateBlock::MovieGateUpdate(void)
{
	float shakeX = 0.0f;
	bool bShaking = false;

	// カウントダウン
	m_movieTime--;

	if (m_movieTime <= 0.0f)
	{
		float t = -m_movieTime; // 揺れ開始からの経過時間

		// 揺れている間だけ
		if (t <= SHAKE_DURATION)
		{
			bShaking = true;

			shakeX = sinf(t * SHAKE_SPEED) * SHAKE_POWER;

			// 少し下げた位置に生成
			D3DXVECTOR3 spawnBase = GetPos();
			spawnBase.y -= 60.0f;

			D3DXVECTOR3 spawnCenter = spawnBase;

			D3DXVECTOR3 spawnLeft = spawnBase;
			spawnLeft.x -= SIDE_OFFSET;

			D3DXVECTOR3 spawnRight = spawnBase;
			spawnRight.x += SIDE_OFFSET;

			// 埃パーティクル生成
			CParticle::Create<CDustParticle>(INIT_VEC3, spawnCenter,
				D3DXCOLOR(0.8f, 0.8f, 0.8f, 0.3f), 90, 1);

			CParticle::Create<CDustParticle>(INIT_VEC3, spawnLeft,
				D3DXCOLOR(0.8f, 0.8f, 0.8f, 0.3f), 90, 1);

			CParticle::Create<CDustParticle>(INIT_VEC3, spawnRight,
				D3DXCOLOR(0.8f, 0.8f, 0.8f, 0.3f), 90, 1);
		}
		else
		{
			shakeX = 0.0f; // 完全停止
		}
	}

	// ジョイパッドの取得
	CInputJoypad* pJoypad = CManager::GetInputJoypad();

	// =========================
	// 振動制御
	// =========================
	if (bShaking && pJoypad)
	{
		// 振動させる
		pJoypad->SetVibration(20000, 20000);
	}
	else
	{
		// 振動停止
		pJoypad->StopVibration();
	}

	// 位置を取得して反映する
	D3DXVECTOR3 pos = GetPos();
	pos.x = m_startPosX + shakeX;

	SetPos(pos);
}


//=============================================================================
// ギアブロックのコンストラクタ
//=============================================================================
CGearBlock::CGearBlock()
{
	// 値のクリア
	m_turnTimer		= DELAY_TIME;	// 回転までの遅延時間
	m_prevTimeEnd	= false;		// タイマーが0になったか
}
//=============================================================================
// ギアブロックのデストラクタ
//=============================================================================
CGearBlock::~CGearBlock()
{
	// なし
}
//=============================================================================
// ギアブロックの更新処理
//=============================================================================
void CGearBlock::Update(void)
{
	// ブロックの更新処理
	CBlock::Update();

	// ゲームシーンだったら
	if (CManager::GetMode() == CScene::MODE_GAME)
	{
		GameGearUpdate();
	}
	// ムービーシーンだったら
	else if (CManager::GetMode() == CScene::MODE_MOVIE)
	{
		MovieGearUpdate();
	}
}
//=============================================================================
// ギアブロックの更新処理
//=============================================================================
void CGearBlock::GameGearUpdate(void)
{
	// 一度だけ通す
	if (!m_prevTimeEnd)
	{
		// 音の取得
		CSound* pSound = CManager::GetSound();

		// ギアSEの再生
		if (pSound)
		{
			pSound->Play(CSound::SOUND_LABEL_GEAR);
		}

		m_prevTimeEnd = true;
	}

	// 向きを取得して、回転させる
	D3DXVECTOR3 rot = GetRot();

	rot.z += ROT_SPEED;

	// 向きを設定する
	SetRot(rot);
}
//=============================================================================
// ギアブロックの更新処理
//=============================================================================
void CGearBlock::MovieGearUpdate(void)
{
	m_turnTimer--;

	// タイマーが0になるまで回転しない
	if (m_turnTimer >= 0)
	{
		return;
	}

	// タイマーが0になったら一回だけ再生する
	bool isTimeEnd = m_turnTimer <= 0;

	if (isTimeEnd && !m_prevTimeEnd)
	{
		// 音の取得
		CSound* pSound = CManager::GetSound();

		// ギアSEの再生
		if (pSound)
		{
			pSound->Play(CSound::SOUND_LABEL_GEAR);
		}
	}

	m_prevTimeEnd = isTimeEnd;

	// 0にしておく
	m_turnTimer = 0;

	// 向きを取得して、回転させる
	D3DXVECTOR3 rot = GetRot();

	rot.z += ROT_SPEED;

	// 向きを設定する
	SetRot(rot);
}
