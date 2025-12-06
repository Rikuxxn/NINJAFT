//=============================================================================
//
// リザルト処理 [result.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "result.h"
#include "input.h"
#include "manager.h"
#include "dummyPlayer.h"

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
CResultTime* CResult::m_pTime = nullptr;					// タイムへのポインタ
int CResult::m_nClearMinutes = 0;
int CResult::m_nClearSeconds = 0;
int CResult::m_clearRankIndex = 0;

//=============================================================================
// コンストラクタ
//=============================================================================
CResult::CResult() : CScene(CScene::MODE_RESULT)
{
	// 値のクリア
	m_pBlockManager = nullptr;
	m_pLight = nullptr;
}
//=============================================================================
// デストラクタ
//=============================================================================
CResult::~CResult()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CResult::Init(void)
{
	// マウスカーソルを表示する
	CManager::GetInputMouse()->SetCursorVisibility(true);

	// ブロックマネージャーの生成
	m_pBlockManager = new CBlockManager;

	// ブロックマネージャーの初期化
	m_pBlockManager->Init();

	// ライトの生成
	m_pLight = new CLight;

	// ライトの初期化
	m_pLight->Init();

	// ライトの再設定処理
	ResetLight();

	//// 背景の生成
	//CBackground::Create(D3DXVECTOR3(960.0f, 540.0f, 0.0f), 960.0f, 540.0f, "data/TEXTURE/resultBG.png");

	// ダミープレイヤーの生成
	CDummyPlayer::Create(D3DXVECTOR3(300.0f, 110.0f, -10.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f), CDummyPlayer::NEUTRAL);

	// 合計秒で扱う
	int totalLimit = 3 * 60 + 0;
	int totalRemain = m_nClearMinutes * 60 + m_nClearSeconds;

	// クリアタイム（経過時間）を計算
	int totalClear = totalLimit - totalRemain;
	if (totalClear < 0)
	{
		totalClear = 0; // 念のため
	}

	// 分と秒に戻す
	int clearMinutes = totalClear / 60;
	int clearSeconds = totalClear % 60;

	// タイムの生成
	m_pTime = CResultTime::Create(clearMinutes, clearSeconds, 1100.0f, 695.0f, 72.0f, 88.0f);

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CResult::Uninit(void)
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
void CResult::Update(void)
{
	// ブロックマネージャーの更新処理
	m_pBlockManager->Update();

	CInputMouse* pInputMouse = CManager::GetInputMouse();
	CInputJoypad* pJoypad = CManager::GetInputJoypad();
	CFade* pFade = CManager::GetFade();

	if (pFade->GetFade() == CFade::FADE_NONE && (pInputMouse->GetTrigger(0) || pJoypad->GetTrigger(CInputJoypad::JOYKEY_A)))
	{
		// ランキング画面に移行
		pFade->SetFade(MODE_RANKING);
	}

#ifdef _DEBUG
	if (pFade->GetFade() == CFade::FADE_NONE && CManager::GetInputKeyboard()->GetTrigger(DIK_RETURN))
	{
		// ランキング画面に移行
		pFade->SetFade(MODE_RANKING);
	}
#endif
}
//=============================================================================
// 描画処理
//=============================================================================
void CResult::Draw(void)
{


}
//=============================================================================
// ライト設定処理
//=============================================================================
void CResult::ResetLight(void)
{
	// ライトを削除しておく
	CLight::Uninit();

	// 暖色・斜め下方向
	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		D3DXCOLOR(1.0f, 0.65f, 0.4f, 1.0f),    // オレンジ系
		D3DXVECTOR3(-0.3f, -0.8f, 0.2f),       // 西日のように斜め
		D3DXVECTOR3(0.0f, 300.0f, 0.0f)
	);

	// 暖色
	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		D3DXCOLOR(1.0f, 0.65f, 0.4f, 1.0f),    // オレンジ系
		D3DXVECTOR3(0.0f, -0.8f, -0.2f),
		D3DXVECTOR3(0.0f, 300.0f, 0.0f)
	);

	// 薄い青
	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		D3DXCOLOR(0.3f, 0.35f, 0.5f, 1.0f),    // 夕方の空の寒色寄り
		D3DXVECTOR3(0.0f, 1.0f, 0.0f),
		D3DXVECTOR3(0.0f, 0.0f, 0.0f)
	);

	// 環境的な補助光
	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		D3DXCOLOR(0.5f, 0.4f, 0.4f, 1.0f),     // 柔らかめの赤系
		D3DXVECTOR3(0.3f, -0.2f, -0.3f),
		D3DXVECTOR3(0.0f, 0.0f, 0.0f)
	);
}
//=============================================================================
// デバイスリセット通知
//=============================================================================
void CResult::OnDeviceReset(void)
{
	// ライトの再設定処理
	ResetLight();
}
//=============================================================================
// サムネイルリリース通知
//=============================================================================
void CResult::ReleaseThumbnail(void)
{
	m_pBlockManager->ReleaseThumbnailRenderTarget();
}
//=============================================================================
// サムネイルリセット通知
//=============================================================================
void CResult::ResetThumbnail(void)
{
	m_pBlockManager->InitThumbnailRenderTarget(CManager::GetRenderer()->GetDevice());
	m_pBlockManager->GenerateThumbnailsForResources(); // キャッシュも再作成
}
