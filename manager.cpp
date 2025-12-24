//=============================================================================
//
// マネージャー処理 [manager.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "manager.h"
#include "renderer.h"
#include "sound.h"
#include "game.h"

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
CRenderer* CManager::m_pRenderer = nullptr;
CInputKeyboard* CManager::m_pInputKeyboard = nullptr;
CInputJoypad* CManager::m_pInputJoypad = nullptr;
CInputMouse* CManager::m_pInputMouse = nullptr;
CSound* CManager::m_pSound = nullptr;
CTexture* CManager::m_pTexture = nullptr;
CCamera* CManager::m_pCamera = nullptr;
CScene* CManager::m_pScene = nullptr;
CFade* CManager::m_pFade = nullptr;
std::unique_ptr<btDiscreteDynamicsWorld> CManager::m_pDynamicsWorld = nullptr;

// 名前空間stdの使用
using namespace std;

//=============================================================================
// コンストラクタ
//=============================================================================
CManager::CManager()
{
	// 値のクリア
	m_fps						= 0;		// FPS値
	m_pBroadphase				= nullptr;	// 衝突判定のクラスへのポインタ
	m_pCollisionConfiguration	= nullptr;	// 衝突検出の設定を管理するクラスへのポインタ
	m_pDispatcher				= nullptr;	// 実際に衝突判定処理を実行するクラスへのポインタ
	m_pSolver					= nullptr;	// 物理シミュレーションの制約ソルバーへのポインタ
}
//=============================================================================
// デストラクタ
//=============================================================================
CManager::~CManager()
{

}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CManager::Init(HINSTANCE hInstance, HWND hWnd)
{
	// レンダラーの生成
	m_pRenderer = new CRenderer;

	// レンダラーの初期化処理
	if (FAILED(m_pRenderer->Init(hWnd, TRUE)))
	{
		return -1;
	}

	// キーボードの生成
	m_pInputKeyboard = new CInputKeyboard;

	// キーボードの初期化処理
	if (FAILED(m_pInputKeyboard->Init(hInstance, hWnd)))
	{
		return -1;
	}

	// ジョイパッドの生成
	m_pInputJoypad = new CInputJoypad;

	// ジョイパッドの初期化処理
	if (FAILED(m_pInputJoypad->Init()))
	{
		return E_FAIL;
	}

	// マウスの生成
	m_pInputMouse = new CInputMouse;

	// マウスの初期化処理
	if (FAILED(m_pInputMouse->Init(hInstance, hWnd)))
	{
		return E_FAIL;
	}

	// サウンドの生成
	m_pSound = new CSound;

	// サウンドの初期化処理
	if (FAILED(m_pSound->Init(hWnd)))
	{
		return E_FAIL;
	}

	// Bullet物理ワールドの生成
	m_pBroadphase = make_unique<btDbvtBroadphase>();
	m_pCollisionConfiguration = make_unique <btDefaultCollisionConfiguration>();
	m_pDispatcher = make_unique <btCollisionDispatcher>(m_pCollisionConfiguration.get());
	m_pSolver = make_unique <btSequentialImpulseConstraintSolver>();

	m_pDynamicsWorld = make_unique <btDiscreteDynamicsWorld>(m_pDispatcher.get(), m_pBroadphase.get(), m_pSolver.get(), m_pCollisionConfiguration.get());

	// 重力を設定
	m_pDynamicsWorld->setGravity(btVector3(0, -9.8f, 0));

	// カメラの生成
	m_pCamera = new CCamera;

	// カメラの初期化処理
	m_pCamera->Init();

	// テクスチャの生成
	m_pTexture = new CTexture;

	// テクスチャの読み込み
	m_pTexture->Load();

	// タイトル画面
	m_pFade = CFade::Create(CScene::MODE_TITLE);

	// タイトル画面
	m_pScene = CScene::Create(CScene::MODE_TITLE);

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CManager::Uninit(void)
{
	// すべてのオブジェクトの破棄
	CObject::ReleaseAll();

	// テクスチャの破棄
	if (m_pTexture != nullptr)
	{
		// 全てのテクスチャの破棄
		m_pTexture->Unload();

		delete m_pTexture;
		m_pTexture = nullptr;
	}

	// キーボードの終了処理
	m_pInputKeyboard->Uninit();

	// ジョイパッドの終了処理
	m_pInputJoypad->Uninit();

	// マウスの終了処理
	m_pInputMouse->Uninit();

	// サウンドの終了処理
	m_pSound->Uninit();

	// キーボードの破棄
	if (m_pInputKeyboard != nullptr)
	{
		// レンダラーの終了処理
		m_pInputKeyboard->Uninit();

		delete m_pInputKeyboard;
		m_pInputKeyboard = nullptr;
	}

	// ジョイパッドの破棄
	if (m_pInputJoypad != nullptr)
	{
		// ジョイパッドの終了処理
		m_pInputJoypad->Uninit();

		delete m_pInputJoypad;
		m_pInputJoypad = nullptr;
	}

	// マウスの破棄
	if (m_pInputMouse != nullptr)
	{
		// マウスの終了処理
		m_pInputMouse->Uninit();

		delete m_pInputMouse;
		m_pInputMouse = nullptr;
	}

	// サウンドの破棄
	if (m_pSound != nullptr)
	{
		// サウンドの終了処理
		m_pSound->Uninit();

		delete m_pSound;
		m_pSound = nullptr;
	}

	// カメラの破棄
	if (m_pCamera != nullptr)
	{
		delete m_pCamera;
		m_pCamera = nullptr;
	}

	// フェードの破棄
	if (m_pFade != nullptr)
	{
		// フェードの終了処理
		m_pFade->Uninit();

		delete m_pFade;
		m_pFade = nullptr;
	}

	// レンダラーの破棄
	if (m_pRenderer != nullptr)
	{
		// レンダラーの終了処理
		m_pRenderer->Uninit();

		delete m_pRenderer;
		m_pRenderer = nullptr;
	}
}
//=============================================================================
// 更新処理
//=============================================================================
void CManager::Update(void)
{
	// キーボードの更新
	m_pInputKeyboard->Update();

	// ジョイパッドの更新
	m_pInputJoypad->Update();

	// マウスの更新
	m_pInputMouse->Update();

	// フェードの更新処理
	if (m_pFade != nullptr)
	{
		// フェードの更新処理
		m_pFade->Update();
	}

	// ポーズ中はゲーム更新しない
	if (CGame::GetisPaused() == true)
	{
		if (m_pInputKeyboard->GetTrigger(DIK_TAB) || m_pInputJoypad->GetTrigger(CInputJoypad::JOYKEY_START))
		{
			CGame::SetEnablePause(false);
		}

		// マウスカーソルを表示にする
		m_pInputMouse->SetCursorVisibility(true);

		// ポーズマネージャーの更新処理
		CGame::GetPauseManager()->Update();

		return;
	}

	m_pDynamicsWorld->stepSimulation((btScalar)m_fps, 1 ,0.16f);

	// カメラの更新
	m_pCamera->Update();

	// レンダラーの更新
	m_pRenderer->Update();
}
//=============================================================================
// 描画処理
//=============================================================================
void CManager::Draw(void)
{
	// カメラの設定
	m_pCamera->SetCamera();

	// レンダラーの描画
	m_pRenderer->Draw(m_fps);
}
//=============================================================================
// モードの設定
//=============================================================================
void CManager::SetMode(CScene::MODE mode)
{
	// カメラの初期化処理
	m_pCamera->Init();

	// サウンドの停止
	m_pSound->Stop();

	if (m_pScene != nullptr)
	{
		// 現在のモード破棄
		m_pScene->Uninit();
	}

	// 全てのオブジェクトを破棄
	CObject::ReleaseAll();

	// ポーズをfalseにしておく
	CGame::SetEnablePause(false);

	// 新しいモードの生成
	m_pScene = CScene::Create(mode);
}
//=============================================================================
// 現在のモードの取得
//=============================================================================
CScene::MODE CManager::GetMode(void)
{
	return m_pScene->GetMode();
}
//=============================================================================
// デバイスリセット通知
//=============================================================================
void CManager::OnDeviceReset(void)
{
	if (m_pScene)
	{
		m_pScene->OnDeviceReset();
	}
}
//=============================================================================
// サムネイルリリース通知
//=============================================================================
void CManager::ReleaseThumbnail(void)
{
	if (m_pScene)
	{
		m_pScene->ReleaseThumbnail();
	}
}
//=============================================================================
// サムネイルリセット通知
//=============================================================================
void CManager::ResetThumbnail(void)
{
	if (m_pScene)
	{
		m_pScene->ResetThumbnail();
	}
}