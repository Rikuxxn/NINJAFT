//=============================================================================
//
// ゲーム処理 [game.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "game.h"
#include "manager.h"
#include "result.h"
#include "particle.h"
#include "charactermanager.h"
#include "player.h"
#include "enemy.h"
#include "ui.h"
#include "meshdome.h"
#include "blocklist.h"
#include "resultsoundcount.h"
#include "grid.h"

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
CPlayer* CGame::m_pPlayer = nullptr;				// プレイヤーへのポインタ
CEnemy* CGame::m_pEnemy = nullptr;					// 敵へのポインタ
CTime* CGame::m_pTime = nullptr;					// タイムへのポインタ
CBlock* CGame::m_pBlock= nullptr;					// ブロックへのポインタ
CBlockManager* CGame::m_pBlockManager= nullptr;		// ブロックマネージャーへのポインタ
CObjectBillboard* CGame::m_pBillboard = nullptr;	// ビルボードへのポインタ
CPauseManager* CGame::m_pPauseManager = nullptr;	// ポーズマネージャーへのポインタ
bool CGame::m_isPaused = false;						// trueならポーズ中
int CGame::m_nSeed = 0;								// マップのシード値

//=============================================================================
// コンストラクタ
//=============================================================================
CGame::CGame() : CScene(CScene::MODE_GAME)
{
	// 値のクリア
	m_pRankingManager = nullptr;
	m_pLight = nullptr;
	m_timer = 0;// パーティクル生成タイマー
	m_startState = StartState::WaitStart;
	m_stateTimer = 0.0f;// UI遅延タイマー
	m_canControl = false;
}
//=============================================================================
// デストラクタ
//=============================================================================
CGame::~CGame()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CGame::Init(void)
{
#ifdef _DEBUG
	// グリッドの生成
	m_pGrid = CGrid::Create();

	// グリッドの初期化
	m_pGrid->Init();
#endif

	// ブロックマネージャーの生成
	m_pBlockManager = new CBlockManager;

	// ブロックマネージャーの初期化
	m_pBlockManager->Init();

	// ライトの生成
	m_pLight = new CLight;

	// ライトの初期化
	m_pLight->Init();

	m_nSeed = (int)time(nullptr);  // シード値をランダム設定

	// マップランダム生成
	m_pBlockManager->GenerateRandomMap(m_nSeed);

	//// 配置情報の読み込み
	//m_pBlockManager->LoadFromJson("data/game_info.json");

	CCharacterManager characterManager;

	// プレイヤーの生成
	m_pPlayer = CPlayer::Create(D3DXVECTOR3(0.0f, 30.0f, -300.0f), D3DXVECTOR3(0.0f, 180.0f, 0.0f));
	characterManager.AddCharacter(m_pPlayer);

	// リーダー敵の生成
	CEnemyLeader* pLeader =
		CEnemy::CreateTyped<CEnemyLeader>(
			D3DXVECTOR3(0.0f, 20.0f, 300.0f),
			D3DXVECTOR3(0.0f, 0.0f, 0.0f)
			);
	m_pEnemy = pLeader;
	characterManager.AddCharacter(pLeader);

	// サブ敵生成
	constexpr int NUM_SUB_ENEMIES = 50;
	std::vector<CEnemy*> subEnemies;
	for (int i = 0; i < NUM_SUB_ENEMIES; i++)
	{
		// リーダーの周囲に配置
		D3DXVECTOR3 offset(
			(rand() % 200 - 100),  // -100～100
			30.0f,
			(rand() % 200 - 100)
		);

		CEnemySub* pSub =
			CEnemy::CreateTyped<CEnemySub>(
				pLeader->GetPos() + offset,
				D3DXVECTOR3(0.0f, 0.0f, 0.0f)
				);

		// リーダーの設定
		pSub->SetLeader(pLeader);

		// サブの設定
		pLeader->SetSub(pSub);

		characterManager.AddCharacter(pSub);
		subEnemies.push_back(pSub);
	}

	// タイムの生成
	m_pTime = CTime::Create(3, 0, 760.0f, 10.0f, 42.0f, 58.0f);

	// メッシュドームの生成
	CMeshDome::Create(D3DXVECTOR3(0.0f, 0.0f, 0.0f), 1000);

	// 任務開始UI生成
	auto mission = CUITexture::Create("data/TEXTURE/ui_mission2.png", 880.0f, 490.0f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 290.0f, 110.0f);

	// 任務失敗UI生成
	auto mission_failure = CUITexture::Create("data/TEXTURE/ui_mission_failure.png", 880.0f, 490.0f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 290.0f, 110.0f);

	// 任務開始UI登録
	CUIManager::GetInstance()->AddUI("Mission", mission);

	// 任務失敗UI登録
	CUIManager::GetInstance()->AddUI("MissionFailure", mission_failure);

	// 生成直後に各UIの設定をする
	mission->Hide();
	mission_failure->Hide();

	m_startState = StartState::WaitStart;
	m_stateTimer = 190;   // 開始時の初期待機
	m_canControl = false;

	// ポーズマネージャーの生成
	m_pPauseManager = new CPauseManager();

	// ポーズマネージャーの初期化
	m_pPauseManager->Init();

	// ランキングマネージャーのインスタンス生成
	m_pRankingManager = make_unique<CRankingManager>();

	// 音の取得
	CSound* pSound = CManager::GetSound();

	// ゲームBGMの再生
	if (pSound)
	{
		pSound->Play(CSound::SOUND_LABEL_GAMEBGM);
	}

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CGame::Uninit(void)
{
	// ブロックマネージャーの破棄
	if (m_pBlockManager != nullptr)
	{
		m_pBlockManager->Uninit();

		delete m_pBlockManager;
		m_pBlockManager = nullptr;
	}

	// ライトの破棄
	if (m_pLight != nullptr)
	{
		delete m_pLight;
		m_pLight = nullptr;
	}

	// ポーズマネージャーの破棄
	if (m_pPauseManager != nullptr)
	{
		// ポーズマネージャーの終了処理
		m_pPauseManager->Uninit();

		delete m_pPauseManager;
		m_pPauseManager = nullptr;
	}
}
//=============================================================================
// 更新処理
//=============================================================================
void CGame::Update(void)
{
	m_timer++;

	if (m_timer >= 15)// 一定間隔で生成
	{// 塵の生成
		// リセット
		m_timer = 0;

		// パーティクル生成
		CParticle::Create<CBlossomParticle>(INIT_VEC3, m_pPlayer->GetPos(), D3DXCOLOR(0.8f, 0.8f, 0.8f, 0.4f), 0, 1);
	}

	CFade* pFade = CManager::GetFade();
	CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();
	CInputJoypad* pJoypad = CManager::GetInputJoypad();

	UpdateLight();

	// TABキーでポーズON/OFF
	if (pKeyboard->GetTrigger(DIK_TAB) || pJoypad->GetTrigger(CInputJoypad::JOYKEY_START))
	{
		// ポーズSE
		CManager::GetSound()->Play(CSound::SOUND_LABEL_PAUSE);

		// ポーズ切り替え前の状態を記録
		bool wasPaused = m_isPaused;

		m_isPaused = !m_isPaused;

		// ポーズ状態に応じて音を制御
		if (m_isPaused && !wasPaused)
		{
			// 一時停止する
			CManager::GetSound()->PauseAll();
		}
		else if (!m_isPaused && wasPaused)
		{
			// 再開する
			CManager::GetSound()->ResumeAll();
		}
	}

	// ブロックマネージャーの更新処理
	m_pBlockManager->Update();

	// UIの更新
	UIUpdate();

	// --- 脱出したか確認 ---
	auto exitBlocks = CBlockManager::GetBlocksOfType<CExitBlock>();

	for (CExitBlock* exit : exitBlocks)
	{
		if (pFade->GetFade() == CFade::FADE_NONE && exit->IsEscape())
		{
			// 宝の獲得数の取得
			int treasureCount = CBuriedTreasureBlock::GetTreasureCount();

			// ランキングに登録
			//m_pRankingManager->AddRecordWithLimit(3, 0, m_pTime->GetMinutes(), m_pTime->GetnSeconds());
			m_pRankingManager->AddItemRecord(treasureCount);

			// 順位のインデックスを取得
			int rankIndex = m_pRankingManager->GetRankIdx();

			// リザルトにセット
			CResult::SetClearRank(rankIndex);// アニメーション用に順位のインデックスを渡す
			CResult::SetClearTime(m_pTime->GetMinutes(), m_pTime->GetnSeconds());

			// 音発生数の取得
			int count = m_pEnemy->GetSoundCount();

			// 宝の獲得数の設定
			CResult::SetTreasureCount(treasureCount);

			// 音発生数の設定
			CResult::SetSoundCount(count);

			// リザルト画面に移行
			pFade->SetFade(MODE_RESULT);
		}
	}

#ifdef _DEBUG
	CInputKeyboard* pInputKeyboard = CManager::GetInputKeyboard();

	if (pFade->GetFade() == CFade::FADE_NONE && pInputKeyboard->GetTrigger(DIK_RETURN))
	{
		// 宝の獲得数の取得
		int treasureCount = CBuriedTreasureBlock::GetTreasureCount();

		// ランキングに登録
		//m_pRankingManager->AddRecordWithLimit(3, 0, m_pTime->GetMinutes(), m_pTime->GetnSeconds());
		m_pRankingManager->AddItemRecord(treasureCount);

		// 順位のインデックスを取得
		int rankIndex = m_pRankingManager->GetRankIdx();

		// リザルトにセット
		CResult::SetClearRank(rankIndex);// アニメーション用に順位のインデックスを渡す
		CResult::SetClearTime(m_pTime->GetMinutes(), m_pTime->GetnSeconds());

		// 音発生数の取得
		int count = m_pEnemy->GetSoundCount();

		// 宝の獲得数の設定
		CResult::SetTreasureCount(treasureCount);

		// 音発生数の設定
		CResult::SetSoundCount(count);

		// リザルト画面に移行
		pFade->SetFade(MODE_RESULT);
	}
#endif
}
//=============================================================================
// ライトの色更新処理
//=============================================================================
void CGame::UpdateLight(void)
{
	float progress = m_pTime->GetProgress(); // 0.0～0.1

	// ======== 各時間帯のメインライト色 ========
	D3DXCOLOR evening(1.0f, 0.65f, 0.35f, 1.0f); // 夕方
	D3DXCOLOR night(0.15f, 0.18f, 0.35f, 1.0f);  // 夜
	D3DXCOLOR morning(0.95f, 0.8f, 0.7f, 1.0f);  // 明け方

	D3DXCOLOR mainColor;

	// ======== 時間帯ごとに補間 ========
	if (progress < 0.30f)
	{// 夕方
		float t = progress / 0.30f;
		D3DXColorLerp(&mainColor, &evening, &night, t);
	}
	else if (progress < 0.90f)
	{// 夜
		float t = (progress - 0.30f) / (0.90f - 0.30f);
		D3DXColorLerp(&mainColor, &night, &morning, t);
	}
	else
	{// 明け方
		float t = (progress - 0.90f) / (1.0f - 0.90f);
		D3DXColorLerp(&mainColor, &morning, &evening, t);
	}

	// ======== 光の向き補間 ========
	D3DXVECTOR3 dirEvening(0.5f, -1.0f, 0.3f);
	D3DXVECTOR3 dirNight(0.0f, -1.0f, 0.0f);
	D3DXVECTOR3 dirMorning(-0.3f, -1.0f, -0.2f);
	D3DXVECTOR3 mainDir;

	if (progress < 0.5f)
	{
		float t = progress / 0.5f;
		D3DXVec3Lerp(&mainDir, &dirEvening, &dirNight, t);
	}
	else
	{
		float t = (progress - 0.5f) / 0.5f;
		D3DXVec3Lerp(&mainDir, &dirNight, &dirMorning, t);
	}
	D3DXVec3Normalize(&mainDir, &mainDir);

	// 再設定
	CLight::Uninit();

	m_pBlockManager->UpdateLight();

	// メインライト
	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		mainColor,
		mainDir,
		D3DXVECTOR3(0.0f, 300.0f, 0.0f)
	);

	// サブライト
	D3DXCOLOR skyEvening(0.4f, 0.45f, 0.8f, 1.0f);
	D3DXCOLOR skyNight(0.1f, 0.15f, 0.3f, 1.0f);
	D3DXCOLOR skyMorning(0.6f, 0.7f, 1.0f, 1.0f);
	D3DXCOLOR skyColor;

	if (progress < 0.5f)
	{
		D3DXColorLerp(&skyColor, &skyEvening, &skyNight, progress / 0.5f);
	}
	else
	{
		D3DXColorLerp(&skyColor, &skyNight, &skyMorning, (progress - 0.5f) / 0.5f);
	}

	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		skyColor,
		D3DXVECTOR3(0.0f, -1.0f, 0.0f),
		D3DXVECTOR3(0.0f, 0.0f, 0.0f)
	);

	// 補助光
	float warmFactor = 1.0f - fabs(progress - 0.5f) * 2.0f;
	warmFactor = max(0.0f, warmFactor);

	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		D3DXCOLOR(0.5f + 0.2f * warmFactor, 0.3f, 0.25f, 1.0f),
		D3DXVECTOR3(-0.3f, 0.0f, -0.7f),
		D3DXVECTOR3(0.0f, 0.0f, 0.0f)
	);
}
//=============================================================================
// 描画処理
//=============================================================================
void CGame::Draw(void)
{
#ifdef _DEBUG
	// グリッドの描画
	m_pGrid->Draw();
#endif
	// ポーズ中だったら
	if (m_isPaused)
	{
		// ポーズマネージャーの描画処理
		m_pPauseManager->Draw();
	}
}
//=============================================================================
// UIの更新処理
//=============================================================================
void CGame::UIUpdate(void)
{
	CFade* pFade = CManager::GetFade();

	switch (m_startState)
	{
	case StartState::WaitStart:
		m_stateTimer--;

		if (m_stateTimer <= 0.0f)
		{
			// 音の取得
			CSound* pSound = CManager::GetSound();

			// 開始SEの再生
			if (pSound)
			{
				pSound->Play(CSound::SOUND_LABEL_START);
			}

			// UI表示
			auto mission = CUIManager::GetInstance()->GetUI("Mission");
			mission->Show();

			m_startState = StartState::Hidden;
			m_stateTimer = 180;  // UI表示時間
		}
		break;

	case StartState::Hidden:
		m_stateTimer--;

		if (m_stateTimer <= 0.0f)
		{
			// UI非表示
			auto mission = CUIManager::GetInstance()->GetUI("Mission");
			mission->FadeOut(60.0f);

			// 操作フラグをtrueにする
			m_pPlayer->SetControlFlag(true);
			m_pEnemy->SetControlFlag(true);

			// タイマーも開始
			m_pTime->SetActiveFlag(true);

			m_startState = StartState::Idle;
		}
		break;

	case StartState::Idle:

		// タイムアップまたはプレイヤーが死んだら
		if (m_pTime->IsTimeUp() || m_pPlayer->IsDead())
		{
			m_stateTimer = 120;
			m_startState = StartState::Failure;
		}
		break;
	case StartState::Failure:// 失敗時
		m_stateTimer--;

		if (m_stateTimer <= 0.0f)
		{
			m_stateTimer = 180;

			// UI表示
			auto mission_failure = CUIManager::GetInstance()->GetUI("MissionFailure");
			mission_failure->FadeIn(120.0f);

			m_startState = StartState::WaitEnd;
		}

		break;
	case StartState::WaitEnd:// 終了までの待機時間
		m_stateTimer--;

		if (pFade->GetFade() == CFade::FADE_NONE && m_stateTimer <= 0.0f)
		{
			// タイトル画面に移行
			pFade->SetFade(MODE_TITLE);
		}

		break;
	}
}
//=============================================================================
// デバイスリセット通知
//=============================================================================
void CGame::OnDeviceReset(void)
{
	// ゲームライトの更新処理
	UpdateLight();
}
//=============================================================================
// ポーズの設定
//=============================================================================
void CGame::SetEnablePause(bool bPause)
{
	m_isPaused = bPause;

	if (bPause)
	{
		// 音を一時停止
		CManager::GetSound()->PauseAll();
	}
	else
	{
		// 音を再開
		CManager::GetSound()->ResumeAll();
	}
}
//=============================================================================
// サムネイルリリース通知
//=============================================================================
void CGame::ReleaseThumbnail(void)
{
	m_pBlockManager->ReleaseThumbnailRenderTarget();
}
//=============================================================================
// サムネイルリセット通知
//=============================================================================
void CGame::ResetThumbnail(void)
{
	m_pBlockManager->InitThumbnailRenderTarget(CManager::GetRenderer()->GetDevice());
	m_pBlockManager->GenerateThumbnailsForResources(); // キャッシュも再作成
}
