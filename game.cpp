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
#include "resultcount.h"
#include "grid.h"
#include "generateMap.h"

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
CTime* CGame::m_pTime = nullptr;					// タイムへのポインタ
CBlockManager* CGame::m_pBlockManager= nullptr;		// ブロックマネージャーへのポインタ
CPauseManager* CGame::m_pPauseManager = nullptr;	// ポーズマネージャーへのポインタ
bool CGame::m_isPaused = false;						// trueならポーズ中
int CGame::m_nSeed = 0;								// マップのシード値

//=============================================================================
// コンストラクタ
//=============================================================================
CGame::CGame() : CScene(CScene::MODE_GAME)
{
	// 値のクリア
	m_pRankingManager	= nullptr;					// ランキングマネージャーへのポインタ
	m_pLight			= nullptr;					// ライトへのポインタ
	m_timer				= 0;						// パーティクル生成タイマー
	m_startState		= StartState::WaitStart;	// UIの状態
	m_stateTimer		= 0.0f;						// UI遅延タイマー
	m_canControl		= false;					// 操作可能フラグ
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

	// 壁などの配置情報の読み込み
	m_pBlockManager->LoadFromJson("data/game_blockinfo.json");

	// ランダムマップ生成
	CGenerateMap::GetInstance()->GenerateRandomMap(m_nSeed);

	// キャラクターマネージャーの生成
	auto& charaMgr = CCharacterManager::GetInstance();

	// プレイヤーの生成
	m_pPlayer = CPlayer::Create(D3DXVECTOR3(0.0f, 30.0f, -320.0f), D3DXVECTOR3(0.0f, 180.0f, 0.0f));
	charaMgr.AddCharacter(m_pPlayer);

	// リーダー敵の生成
	CEnemyLeader* pLeader =
		CEnemy::CreateTyped<CEnemyLeader>(
			D3DXVECTOR3(0.0f, 20.0f, 300.0f),
			D3DXVECTOR3(0.0f, 0.0f, 0.0f)
			);
	m_pEnemy = pLeader;

	// キャラクターの追加
	charaMgr.AddCharacter(pLeader);

	// サブ敵生成
	constexpr int NUM_SUB_ENEMIES = 30;
	std::vector<CEnemy*> subEnemies;
	for (int i = 0; i < NUM_SUB_ENEMIES; i++)
	{
		// リーダーの周囲に配置
		D3DXVECTOR3 offset(
			(float)(rand() % 200 - 100),  // -100～100
			50.0f,
			(float)(rand() % 200 - 100)
		);

		CEnemySub* pSub =
			CEnemy::CreateTyped<CEnemySub>(
				pLeader->GetPos() + offset,
				D3DXVECTOR3(0.0f, 0.0f, 0.0f)
				);

		// キャラクターの追加
		charaMgr.AddCharacter(pSub);
		subEnemies.push_back(pSub);
	}

	// タイムの生成
	m_pTime = CTime::Create(3, 0, 760.0f, 10.0f, 42.0f, 58.0f, false);

	// メッシュドームの生成
	CMeshDome::Create(D3DXVECTOR3(0.0f, 0.0f, 0.0f), 1000);

	// ルールUI生成
	auto rule = CUITexture::Create("data/TEXTURE/ui_rule.png", 880.0f, 490.0f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 290.0f, 110.0f);

	// 任務失敗UI生成
	auto mission_failure = CUITexture::Create("data/TEXTURE/ui_mission_failure.png", 880.0f, 490.0f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 290.0f, 110.0f);

	// 脱出UI生成
	auto escape_xinput = CUITexture::Create("data/TEXTURE/ui_escape_xinput.png", 880.0f, 820.0f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 165.0f, 60.0f);
	auto escape_keyboard = CUITexture::Create("data/TEXTURE/ui_escape_keyboard.png", 880.0f, 820.0f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 165.0f, 60.0f);

	// ルールUI登録
	CUIManager::GetInstance()->AddUI("Rule", rule);

	// 任務失敗UI登録
	CUIManager::GetInstance()->AddUI("MissionFailure", mission_failure);

	// 脱出UI登録
	CUIManager::GetInstance()->AddUI("Escape_XInput", escape_xinput);
	CUIManager::GetInstance()->AddUI("Escape_Keyboard", escape_keyboard);

	// 生成直後に各UIの設定をする
	rule->Hide();
	mission_failure->Hide();
	escape_xinput->Hide();
	escape_keyboard->Hide();

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
	// キャラクターマネージャーの破棄
	CCharacterManager::GetInstance().Destroy();

	//マップジェネレーターのリストのクリア
	CGenerateMap::GetInstance()->Uninit();

	// ジョイパッドの取得
	CInputJoypad* pJoypad = CManager::GetInputJoypad();

	// 振動停止
	pJoypad->StopVibration();

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
	{// 桜の生成
		// リセット
		m_timer = 0;

		// プレイヤーの位置を基準に生成
		D3DXVECTOR3 playerPos = m_pPlayer->GetPos();

		// パーティクル生成
		CParticle::Create<CBlossomParticle>(INIT_VEC3, playerPos, D3DXCOLOR(0.8f, 0.8f, 0.8f, 0.4f), 0, 1);
	}

	CFade* pFade = CManager::GetFade();
	CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();
	CInputJoypad* pJoypad = CManager::GetInputJoypad();

	UpdateLight();

	// TABキーでポーズON/OFF
	if (pKeyboard->GetTrigger(DIK_TAB) || pJoypad->GetTrigger(CInputJoypad::JOYKEY_START))
	{
		// ポーズ切り替え前の状態を記録
		bool wasPaused = m_isPaused;

		m_isPaused = !m_isPaused;

		// ポーズ状態に応じて音を制御
		if (m_isPaused && !wasPaused)
		{
			// 振動停止
			pJoypad->StopVibration();

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

	// 脱出UIの取得
	auto escape_xinput = CUIManager::GetInstance()->GetUI("Escape_XInput");
	auto escape_keyboard = CUIManager::GetInstance()->GetUI("Escape_Keyboard");

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
			//CResult::SetClearTime(m_pTime->GetMinutes(), m_pTime->GetnSeconds());

			// 音発生数の取得
			int count = m_pEnemy->GetSoundCount();

			// 発見数の取得
			int insightCount = m_pEnemy->GetInsightCount();

			// 宝の獲得数の設定
			CResult::SetTreasureCount(treasureCount);

			// 音発生数の設定
			CResult::SetSoundCount(count);

			// 発見数の設定
			CResult::SetInsightCount(insightCount);

			// リザルト画面に移行
			pFade->SetFade(MODE_RESULT);
		}

		// 範囲内のとき
		if (exit->IsIn())
		{
			// 入力デバイスに応じてUIを切り替える
			if (pJoypad->GetAnyTrigger() || pJoypad->GetStick())
			{
				// 表示
				escape_keyboard->Hide();
				escape_xinput->Show();
			}
			else if (pKeyboard->GetAnyKeyTrigger())
			{
				// 表示
				escape_xinput->Hide();
				escape_keyboard->Show();
			}
		}
		else
		{
			// 非表示
			escape_xinput->Hide();
			escape_keyboard->Hide();
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
		//CResult::SetClearTime(m_pTime->GetMinutes(), m_pTime->GetnSeconds());

		// 音発生数の取得
		int count = m_pEnemy->GetSoundCount();

		// 発見数の取得
		int insightCount = m_pEnemy->GetInsightCount();

		// 宝の獲得数の設定
		CResult::SetTreasureCount(treasureCount);

		// 音発生数の設定
		CResult::SetSoundCount(count);

		// 発見数の設定
		CResult::SetInsightCount(insightCount);

		// リザルト画面に移行
		pFade->SetFade(MODE_RESULT);
	}
#endif
}
//=============================================================================
// ライト更新処理
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

	// UIの取得
	auto rule = CUIManager::GetInstance()->GetUI("Rule");
	auto mission_failure = CUIManager::GetInstance()->GetUI("MissionFailure");

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
			rule->Show();

			m_startState = StartState::Hidden;
			m_stateTimer = 180;  // UI表示時間
		}
		break;

	case StartState::Hidden:
		m_stateTimer--;

		if (m_stateTimer <= 0.0f)
		{
			// UI非表示
			rule->FadeOut(60.0f);

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
			// タイムアップ時に死亡判定にする
			m_pPlayer->SetIsDead(true);

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
