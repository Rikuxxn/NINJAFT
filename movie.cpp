//=============================================================================
//
// ムービー処理 [movie.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "movie.h"
#include "manager.h"
#include "meshdome.h"
#include "time.h"
#include "particle.h"
#include "ui.h"
#include "dummyPlayer.h"

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
CTime* CMovie::m_pTime = nullptr;					// タイムへのポインタ

//=============================================================================
// コンストラクタ
//=============================================================================
CMovie::CMovie() : CScene(CScene::MODE_MOVIE)
{
	// 値のクリア
	m_pBlockManager = nullptr;
	m_pLight		= nullptr;
	m_timer			= 0;
	m_particleTimer = 0;
	m_pDummyPlayer	= nullptr;
	m_smokeTimer	= 0;
	m_smokeActive	= false;
	m_delayTime		= 0;
}
//=============================================================================
// デストラクタ
//=============================================================================
CMovie::~CMovie()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CMovie::Init(void)
{
	// ブロックマネージャーの生成
	m_pBlockManager = new CBlockManager;

	// ブロックマネージャーの初期化
	m_pBlockManager->Init();

	// ライトの生成
	m_pLight = new CLight;

	// ライトの初期化
	m_pLight->Init();

	// 配置情報の読み込み
	m_pBlockManager->LoadFromJson("data/movie_blockinfo.json");

	// タイムの生成
	m_pTime = CTime::Create(3, 0, 760.0f, 10.0f, 42.0f, 58.0f, false);

	// ダミープレイヤーの生成
	m_pDummyPlayer = CDummyPlayer::Create(D3DXVECTOR3(0.0f, 0.0f, 700.0f),
		D3DXVECTOR3(0.0f, 0.0f, 0.0f), CDummyPlayer::APPEARANCE);
	m_pDummyPlayer->SetVisibleFlag(false);

	m_smokeTimer = 15;
	m_delayTime = 120;
	m_smokeActive = true;

	// メッシュドームの生成
	CMeshDome::Create(D3DXVECTOR3(0.0f, 0.0f, 0.0f), 1000);

	// カメラの取得
	CCamera* pCamera = CManager::GetCamera();

	// ムービーカメラの設定
	pCamera->SetCamMode(CCamera::MODE_MOVIE);
	std::vector<CCamera::CameraKeyFrame> movie;

	movie.push_back({
	D3DXVECTOR3(-102.5f, 76.0f, 598.5f),
	D3DXVECTOR3(172.0f, 37.0f, 846.5f),
	D3DXVECTOR3(0.11f, -2.31f, 0.0f),
	0.0f,
	0,
	230
		});

	movie.push_back({
	D3DXVECTOR3(0.0f, 65.5f, 153.5f),
	D3DXVECTOR3(236.0f, -43.5f, 815.5f),
	D3DXVECTOR3(0.17f, -2.65f, 0.0f),
	0.0f,
	180,
	0
		});

	movie.push_back({
	D3DXVECTOR3(-311.9f, 105.2f, 216.5f),
	D3DXVECTOR3(121.2f, -37.2f, 864.0f),
	D3DXVECTOR3(0.23f, -2.55f, 0.0f),
	0.0f,
	180,
	0
		});

	pCamera->SetMovieCamera(movie);

	// レターボックスUI生成
	auto topBar = CUITexture::Create(nullptr, 0.73f, 0.1f, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f), 0.7f, 0.1f);
	auto bottomBar = CUITexture::Create(nullptr, 0.73f, 1.0f, D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f), 0.7f, 0.1f);

	// スキップUI生成
	auto skip_xinput = CUITexture::Create("data/TEXTURE/ui_skip_xinput.png", 0.9f, 0.95f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.09f, 0.06f);
	auto skip_keyboard = CUITexture::Create("data/TEXTURE/ui_skip_keyboard.png", 0.9f, 0.95f, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 0.09f, 0.06f);

	// レターボックスUI登録
	CUIManager::GetInstance()->AddUI("MovieTopBar", topBar);
	CUIManager::GetInstance()->AddUI("MovieBottomBar", bottomBar);

	// スキップUI登録
	CUIManager::GetInstance()->AddUI("Skip_XInput", skip_xinput);
	CUIManager::GetInstance()->AddUI("Skip_Keyboard", skip_keyboard);

	// UI初期設定
	skip_xinput->Show();
	skip_keyboard->Hide();
	
	float barHeightPx =
		CManager::GetRenderer()->GetBackBufferHeight();

	// レターボックスをスライドインさせる
	topBar->SlideIn(D3DXVECTOR3(0.0f, -barHeightPx, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), 60.0f);
	bottomBar->SlideIn(D3DXVECTOR3(0.0f, barHeightPx, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), 60.0f);

	// 音の取得
	CSound* pSound = CManager::GetSound();

	// チュートリアルBGMの再生
	if (pSound)
	{
		pSound->Play(CSound::SOUND_LABEL_TUTORIALBGM);
	}

	// 画面遷移タイマーを設定
	m_timer = FADE_TIME;

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CMovie::Uninit(void)
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
}
//=============================================================================
// 更新処理
//=============================================================================
void CMovie::Update(void)
{
	m_particleTimer++;

	if (m_particleTimer >= 15)// 一定間隔で生成
	{// 桜の生成
		// リセット
		m_particleTimer = 0;

		// カメラの位置を取得して視点を基準に生成
		D3DXVECTOR3 camPosR = CManager::GetCamera()->GetPosR();

		// パーティクル生成
		CParticle::Create<CBlossomParticle>(INIT_VEC3, camPosR, D3DXCOLOR(0.8f, 0.8f, 0.8f, 0.4f), 0, 1);
	}

	// 入力処理の取得
	CFade* pFade = CManager::GetFade();
	CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();
	CInputJoypad* pJoypad = CManager::GetInputJoypad();

	// UIの取得
	auto skip_xinput = CUIManager::GetInstance()->GetUI("Skip_XInput");
	auto skip_keyboard = CUIManager::GetInstance()->GetUI("Skip_Keyboard");

	// 入力デバイスに応じてUIを切り替える
	if (pJoypad->GetAnyTrigger() || pJoypad->GetStick())
	{
		// キーボード表示をOFFにしてXInput表示
		skip_keyboard->Hide();

		skip_xinput->Show();
	}
	else if (pKeyboard->GetAnyKeyTrigger())
	{
		// XInput表示をOFFにしてキーボード表示
		skip_xinput->Hide();

		skip_keyboard->Show();
	}

	// ライトの更新処理
	UpdateLight();

	// ブロックマネージャーの更新処理
	m_pBlockManager->Update();

	// スキップ
	if (pFade->GetFade() == CFade::FADE_NONE && 
		(pKeyboard->GetTrigger(DIK_TAB) || pJoypad->GetTrigger(CInputJoypad::JOYKEY_START)))
	{
		// ゲーム画面に移行
		pFade->SetFade(MODE_GAME);
	}

	m_delayTime--;

	if (m_smokeActive && m_delayTime <= 0)
	{
		m_delayTime = 0;

		for (int i = 0; i < 3; i++)
		{
			//D3DXVECTOR3 pos = m_pDummyPlayer->GetPos();
			D3DXVECTOR3 pos = D3DXVECTOR3(0.0f, 0.0f, 700.0f);
			pos.y += i * 30.0f;

			CParticle::Create<CSmokeParticle>(
				INIT_VEC3, pos,
				D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f),
				120, 8
				);
		}

		if (--m_smokeTimer <= 0)
		{
			// ダミープレイヤー可視フラグをONにする
			m_pDummyPlayer->SetVisibleFlag(true);

			// 出現モーションにする
			m_pDummyPlayer->GetMotion()->StartBlendMotion(CDummyPlayer::APPEARANCE, 10);

			m_smokeActive = false;
		}
	}

	// 出現モーションが終わったらダッシュモーションにする
	if (m_pDummyPlayer->GetMotion()->IsCurrentMotionEnd(CDummyPlayer::APPEARANCE))
	{
		// 移動量の設定
		m_pDummyPlayer->SetMove(D3DXVECTOR3(0.0f, 0.0f, -4.0f));

		// ダッシュモーションにする
		m_pDummyPlayer->GetMotion()->StartBlendMotion(CDummyPlayer::DUSH, 10);
	}

	// カウントダウン
	m_timer--;

	// 一定時間経過したら画面遷移
	if (m_timer <= 0)
	{
		m_timer = 0;


		if (pFade->GetFade() == CFade::FADE_NONE)
		{
			// 振動停止
			pJoypad->StopVibration();

			// ゲーム画面に移行
			pFade->SetFade(MODE_GAME);
		}
	}

#ifdef _DEBUG
	CInputKeyboard* pInputKeyboard = CManager::GetInputKeyboard();

	if (pFade->GetFade() == CFade::FADE_NONE && pInputKeyboard->GetTrigger(DIK_RETURN))
	{
		// ゲーム画面に移行
		pFade->SetFade(MODE_GAME);
	}
#endif

}
//=============================================================================
// ライト更新処理
//=============================================================================
void CMovie::UpdateLight(void)
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
	warmFactor = std::max(0.0f, warmFactor);

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
void CMovie::Draw(void)
{


}
//=============================================================================
// デバイスリセット通知
//=============================================================================
void CMovie::OnDeviceReset(void)
{
	// ゲームライトの更新処理
	UpdateLight();
}
//=============================================================================
// サムネイルリリース通知
//=============================================================================
void CMovie::ReleaseThumbnail(void)
{
	m_pBlockManager->ReleaseThumbnailRenderTarget();
}
//=============================================================================
// サムネイルリセット通知
//=============================================================================
void CMovie::ResetThumbnail(void)
{
	m_pBlockManager->InitThumbnailRenderTarget(CManager::GetRenderer()->GetDevice());
	m_pBlockManager->GenerateThumbnailsForResources(); // キャッシュも再作成
}
