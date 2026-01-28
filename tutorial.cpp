//=============================================================================
//
// チュートリアル処理 [tutorial.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "tutorial.h"
#include "manager.h"
#include "result.h"
#include "particle.h"
#include "charactermanager.h"
#include "player.h"
#include "meshdome.h"
#include "blocklist.h"
#include "ui.h"
#include "generateMap.h"


//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
CTime* CTutorial::m_pTime = nullptr;					// タイムへのポインタ
CBlock* CTutorial::m_pBlock = nullptr;					// ブロックへのポインタ
CBlockManager* CTutorial::m_pBlockManager = nullptr;	// ブロックマネージャーへのポインタ

//=============================================================================
// コンストラクタ
//=============================================================================
CTutorial::CTutorial() : CScene(CScene::MODE_TUTORIAL)
{
	// 値のクリア
	m_pLight	= nullptr;	// ライトへのポインタ
	m_timer		= 0;		// タイマー
}
//=============================================================================
// デストラクタ
//=============================================================================
CTutorial::~CTutorial()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CTutorial::Init(void)
{
	// ブロックマネージャーの生成
	m_pBlockManager = new CBlockManager;

	// ブロックマネージャーの初期化
	m_pBlockManager->Init();

	// ライトの生成
	m_pLight = new CLight;

	// ライトの初期化
	m_pLight->Init();

	// ライトの設定処理
	ResetLight();

	// 配置情報の読み込み
	m_pBlockManager->LoadFromJson("data/tutorial_blockinfo.json");

	// ランダム地形生成
	CGenerateMap::GetInstance()->GenerateRandomTerrain(TUTORIAL_SEED);

	// キャラクターマネージャーの生成
	auto& charaMgr = CCharacterManager::GetInstance();

	// プレイヤーの生成
	m_pPlayer = CPlayer::Create(D3DXVECTOR3(0.0f, 30.0f, -300.0f), D3DXVECTOR3(0.0f, 180.0f, 0.0f));
	charaMgr.AddCharacter(m_pPlayer);

	// タイムの生成
	m_pTime = CTime::Create(3, 0, 760.0f, 10.0f, 42.0f, 58.0f, false);

	// メッシュドームの生成
	CMeshDome::Create(D3DXVECTOR3(0.0f, 0.0f, 0.0f), 1000);

	// 「チュートリアル」UI生成
	auto tutorial = CUITexture::Create("data/TEXTURE/ui_tutorial.png", 0.57f, 0.57f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.15f, 0.12f);

	// スキップUI生成
	auto skip_xinput = CUITexture::Create("data/TEXTURE/ui_skip_xinput.png", 0.9f, 0.9f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.09f, 0.06f);
	auto skip_keyboard = CUITexture::Create("data/TEXTURE/ui_skip_keyboard.png", 0.9f, 0.9f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.09f, 0.06f);

	// 開始UI生成
	auto start_xinput = CUITexture::Create("data/TEXTURE/ui_mission_start_xinput.png", 0.57f, 0.85f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, 0.06f);
	auto start_keyboard = CUITexture::Create("data/TEXTURE/ui_mission_start_keyboard.png", 0.57f, 0.85f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.1f, 0.06f);

	// ルールUI生成
	auto rule_1 = CUITexture::Create("data/TEXTURE/ui_rule1.png", 0.18f, 0.2f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.12f, 0.05f);
	auto rule_2 = CUITexture::Create("data/TEXTURE/ui_rule2.png", 0.18f, 0.35f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.12f, 0.05f);
	auto rule_3 = CUITexture::Create("data/TEXTURE/ui_rule3.png", 0.18f, 0.5f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.12f, 0.05f);

	// 操作UI生成
	auto dush_xinput = CUITexture::Create("data/TEXTURE/ui_operation_dush_xinput.png", 0.95f, 0.2f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.12f, 0.045f);
	auto stealth_xinput = CUITexture::Create("data/TEXTURE/ui_operation_stealth_xinput.png", 0.95f, 0.35f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.12f, 0.045f);
	auto dush_keyboard = CUITexture::Create("data/TEXTURE/ui_operation_dush_keyboard.png", 0.95f, 0.2f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.12f, 0.045f);
	auto stealth_keyboard = CUITexture::Create("data/TEXTURE/ui_operation_stealth_keyboard.png", 0.95f, 0.35f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.12f, 0.045f);

	// 「チュートリアル」UI登録
	CUIManager::GetInstance()->AddUI("Tutorial", tutorial);

	// スキップUI登録
	CUIManager::GetInstance()->AddUI("Skip_XInput", skip_xinput);
	CUIManager::GetInstance()->AddUI("Skip_Keyboard", skip_keyboard);

	// 開始UI登録
	CUIManager::GetInstance()->AddUI("Start_XInput", start_xinput);
	CUIManager::GetInstance()->AddUI("Start_Keyboard", start_keyboard);

	// ルールUI登録
	CUIManager::GetInstance()->AddUI("Rule_1", rule_1);
	CUIManager::GetInstance()->AddUI("Rule_2", rule_2);
	CUIManager::GetInstance()->AddUI("Rule_3", rule_3);

	// 操作UI登録
	CUIManager::GetInstance()->AddUI("Dush_xinput", dush_xinput);
	CUIManager::GetInstance()->AddUI("Stealth_xinput", stealth_xinput);
	CUIManager::GetInstance()->AddUI("Dush_keyboard", dush_keyboard);
	CUIManager::GetInstance()->AddUI("Stealth_keyboard", stealth_keyboard);

	// UI初期設定
	tutorial->Hide();
	skip_xinput->Show();
	skip_keyboard->Hide();
	start_xinput->Hide();
	start_keyboard->Hide();
	dush_xinput->Show();
	stealth_xinput->Show();
	dush_keyboard->Hide();
	stealth_keyboard->Hide();

	m_startState = StartState::WaitStart;
	m_stateTimer = 190;   // 開始時の初期待機
	m_canControl = false;

	// 音の取得
	CSound* pSound = CManager::GetSound();

	// チュートリアルBGMの再生
	if (pSound)
	{
		pSound->Play(CSound::SOUND_LABEL_TUTORIALBGM);
	}

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CTutorial::Uninit(void)
{
	// キャラクターマネージャーの破棄
	CCharacterManager::GetInstance().Destroy();

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
}
//=============================================================================
// 更新処理
//=============================================================================
void CTutorial::Update(void)
{
	m_timer++;

	if (m_timer >= BLOSSOM_INTERVAL)// 一定間隔で生成
	{// 桜の生成
		// リセット
		m_timer = 0;

		// プレイヤーの位置を基準に生成
		D3DXVECTOR3 playerPos(0.0f, 0.0f, 0.0f);

		if (m_pPlayer)
		{
			// プレイヤーの位置を取得
			playerPos = m_pPlayer->GetPos();
		}

		// パーティクル生成
		CParticle::Create<CBlossomParticle>(INIT_VEC3, m_pPlayer->GetPos(), D3DXCOLOR(0.8f, 0.8f, 0.8f, 0.8f), 0, 1);
	}

	CFade* pFade = CManager::GetFade();
	CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();
	CInputJoypad* pJoypad = CManager::GetInputJoypad();

	// UIの取得
	auto skip_xinput = CUIManager::GetInstance()->GetUI("Skip_XInput");
	auto skip_keyboard = CUIManager::GetInstance()->GetUI("Skip_Keyboard");
	auto dush_xinput = CUIManager::GetInstance()->GetUI("Dush_xinput");
	auto dush_keyboard = CUIManager::GetInstance()->GetUI("Dush_keyboard");
	auto stealth_xinput = CUIManager::GetInstance()->GetUI("Stealth_xinput");
	auto stealth_keyboard = CUIManager::GetInstance()->GetUI("Stealth_keyboard");

	// 入力デバイスに応じてUIを切り替える
	if (pJoypad->GetAnyTrigger() || pJoypad->GetStick())
	{
		// キーボード表示をOFFにしてXInput表示
		skip_keyboard->Hide();
		dush_keyboard->Hide();
		stealth_keyboard->Hide();

		skip_xinput->Show();
		dush_xinput->Show();
		stealth_xinput->Show();
	}
	else if (pKeyboard->GetAnyKeyTrigger())
	{
		// XInput表示をOFFにしてキーボード表示
		skip_xinput->Hide();
		dush_xinput->Hide();
		stealth_xinput->Hide();

		skip_keyboard->Show();
		dush_keyboard->Show();
		stealth_keyboard->Show();
	}

	if (m_pPlayer)
	{
		// UIの更新
		UIUpdate();
	}

	// ブロックマネージャーの更新処理
	m_pBlockManager->Update();

	// 任意のボタンを押したとき
	if (pFade->GetFade() == CFade::FADE_NONE &&
		(pKeyboard->GetTrigger(DIK_TAB) || pJoypad->GetTrigger(CInputJoypad::JOYKEY_START)))
	{
		// ムービー画面に移行
		pFade->SetFade(MODE_MOVIE);
	}

	// 開始UIの取得
	auto start_xinput = CUIManager::GetInstance()->GetUI("Start_XInput");
	auto start_keyboard = CUIManager::GetInstance()->GetUI("Start_Keyboard");

	// --- 脱出したか、範囲内か確認 ---
	auto exitBlocks = CBlockManager::GetBlocksOfType<CExitBlock>();

	for (CExitBlock* exit : exitBlocks)
	{
		if (pFade->GetFade() == CFade::FADE_NONE && exit->IsEscape())
		{
			// ムービー画面に移行
			pFade->SetFade(MODE_MOVIE);
		}

		// 範囲内のとき
		if (exit->IsIn())
		{
			// 入力デバイスに応じてUIを切り替える
			if (pJoypad->GetAnyTrigger() || pJoypad->GetStick())
			{
				// 表示
				start_keyboard->Hide();
				start_xinput->Show();
			}
			else if (pKeyboard->GetAnyKeyTrigger())
			{
				// 表示
				start_xinput->Hide();
				start_keyboard->Show();
			}
		}
		else
		{
			// 非表示
			start_xinput->Hide();
			start_keyboard->Hide();
		}
	}

#ifdef _DEBUG
	CInputKeyboard* pInputKeyboard = CManager::GetInputKeyboard();

	if (pFade->GetFade() == CFade::FADE_NONE && pInputKeyboard->GetTrigger(DIK_RETURN))
	{
		// ゲーム画面に移行
		pFade->SetFade(MODE_MOVIE);
	}
#endif

}
//=============================================================================
// ライトの設定処理
//=============================================================================
void CTutorial::ResetLight(void)
{
	// メインライト色
	D3DXCOLOR evening(1.0f, 0.65f, 0.35f, 1.0f); // 夕方

	// 光の向き
	D3DXVECTOR3 dirEvening(0.5f, -1.0f, 0.3f);

	// 再設定
	CLight::Uninit();

	// ブロックマネージャーの更新処理
	m_pBlockManager->UpdateLight();

	// メインライト
	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		evening,
		dirEvening,
		D3DXVECTOR3(0.0f, 300.0f, 0.0f)
	);

	// サブライト
	D3DXCOLOR skyEvening(0.4f, 0.45f, 0.8f, 1.0f);

	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		skyEvening,
		D3DXVECTOR3(0.0f, -1.0f, 0.0f),
		D3DXVECTOR3(0.0f, 0.0f, 0.0f)
	);

	// 補助光
	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		D3DXCOLOR(0.7f, 0.3f, 0.25f, 1.0f),
		D3DXVECTOR3(-0.3f, 0.0f, -0.7f),
		D3DXVECTOR3(0.0f, 0.0f, 0.0f)
	);
}
//=============================================================================
// UIの更新処理
//=============================================================================
void CTutorial::UIUpdate(void)
{
	// UIの取得
	auto tutorial = CUIManager::GetInstance()->GetUI("Tutorial");

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
			tutorial->Show();

			m_startState = StartState::Hidden;
			m_stateTimer = 180;  // UI表示時間
		}
		break;

	case StartState::Hidden:
		m_stateTimer--;

		if (m_stateTimer <= 0.0f)
		{
			// UI非表示
			tutorial->FadeOut(60.0f);

			// 操作フラグをtrueにする
			m_pPlayer->SetControlFlag(true);

			m_startState = StartState::Idle;
		}
		break;

	case StartState::Idle:

		break;
	}
}
//=============================================================================
// 描画処理
//=============================================================================
void CTutorial::Draw(void)
{

}
//=============================================================================
// デバイスリセット通知
//=============================================================================
void CTutorial::OnDeviceReset(void)
{
	// ライトの設定処理
	ResetLight();
}
//=============================================================================
// サムネイルリリース通知
//=============================================================================
void CTutorial::ReleaseThumbnail(void)
{
	m_pBlockManager->ReleaseThumbnailRenderTarget();
}
//=============================================================================
// サムネイルリセット通知
//=============================================================================
void CTutorial::ResetThumbnail(void)
{
	m_pBlockManager->InitThumbnailRenderTarget(CManager::GetRenderer()->GetDevice());
	m_pBlockManager->GenerateThumbnailsForResources(); // キャッシュも再作成
}

