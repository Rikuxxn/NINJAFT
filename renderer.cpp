//=============================================================================
//
// レンダリング処理 [renderer.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "renderer.h"
#include "object.h"
#include "manager.h"
#include "imguimaneger.h"
#include "guiInfo.h"
#include "ui.h"
#include "debugproc.h"
#include "debugproc3D.h"
#include "game.h"

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
CDebugProc* CRenderer::m_pDebug = nullptr;
CDebugProc3D* CRenderer::m_pDebug3D = nullptr;
int CRenderer::m_nFPS = 0;

//=============================================================================
// コンストラクタ
//=============================================================================
CRenderer::CRenderer()
{
	// 値のクリア
	m_pD3D			= nullptr;
	m_pD3DDevice	= nullptr;
	m_ResizeWidth	= 0;
	m_ResizeHeight	= 0;
	m_d3dpp			= {};
	m_bgCol			= INIT_XCOL;
}
//=============================================================================
// デストラクタ
//=============================================================================
CRenderer::~CRenderer()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CRenderer::Init(HWND hWnd, BOOL bWindow)
{
	D3DDISPLAYMODE d3ddm;			// ディスプレイモード

	// DirectX3Dオブジェクトの生成
	m_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

	if (m_pD3D == nullptr)
	{
		return E_FAIL;
	}

	// 現在のディスプレイモードを取得
	if (FAILED(m_pD3D->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
	{
		return E_FAIL;
	}

	// デバイスのプレゼンテーションパラメータの設定
	ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));								// パラメータのゼロクリア

	m_d3dpp.BackBufferWidth				= SCREEN_WIDTH;					// ゲーム画面サイズ（幅）
	m_d3dpp.BackBufferHeight			= SCREEN_HEIGHT;				// ゲーム画面サイズ（高さ）
	m_d3dpp.BackBufferFormat			= d3ddm.Format;					// バックバッファの形式
	m_d3dpp.BackBufferCount				= 1;							// バックバッファの数
	m_d3dpp.SwapEffect					= D3DSWAPEFFECT_DISCARD;		// ダブルバッファの切り替え
	m_d3dpp.EnableAutoDepthStencil		= TRUE;							// デプスバッファとステンシルバッファを作成
	m_d3dpp.AutoDepthStencilFormat		= D3DFMT_D24S8;					// デプスバッファとして16bitを使う
	m_d3dpp.Windowed					= bWindow;						// ウインドウモード
	m_d3dpp.FullScreen_RefreshRateInHz	= D3DPRESENT_RATE_DEFAULT;		// リフレッシュレート
	m_d3dpp.PresentationInterval		= D3DPRESENT_INTERVAL_DEFAULT;	// インターバル

	// DirectX3Dデバイスの生成
	if (FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		hWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&m_d3dpp,
		&m_pD3DDevice)))
	{
		// DirectX3Dデバイスの生成
		if (FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			hWnd,
			D3DCREATE_SOFTWARE_VERTEXPROCESSING,
			&m_d3dpp,
			&m_pD3DDevice)))
		{
			// DirectX3Dデバイスの生成
			if (FAILED(m_pD3D->CreateDevice(D3DADAPTER_DEFAULT,
				D3DDEVTYPE_REF,
				hWnd,
				D3DCREATE_SOFTWARE_VERTEXPROCESSING,
				&m_d3dpp,
				&m_pD3DDevice)))
			{
				return E_FAIL;
			}
		}
	}

	// レンダーステートの設定
	m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// サンプラーステートの設定
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	// テクスチャステージステートの設定
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

	// デバッグフォントの初期化
	m_pDebug->Init();

	// 3Dデバッグ表示の初期化
	m_pDebug3D->Init();

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CRenderer::Uninit(void)
{
	// デバッグフォントの終了処理
	m_pDebug->Uninit();

	// 3Dデバッグ表示の終了処理
	m_pDebug3D->Uninit();

	// Direct3Dデバイスの破棄
	if (m_pD3DDevice != nullptr)
	{
		m_pD3DDevice->Release();
		m_pD3DDevice = nullptr;
	}

	// Direct3Dオブジェクトの破棄
	if (m_pD3D != nullptr)
	{
		m_pD3D->Release();
		m_pD3D = nullptr;
	}
}
//=============================================================================
// 更新処理
//=============================================================================
void CRenderer::Update(void)
{
	// すべてのオブジェクトの更新処理
	CObject::UpdateAll();

#ifdef _DEBUG

	// 場所
	CImGuiManager::Instance().SetPosImgui(ImVec2(20.0f, 20.0f));

	// サイズ
	CImGuiManager::Instance().SetSizeImgui(ImVec2(420.0f, 500.0f));

	// 最初のGui
	CImGuiManager::Instance().StartImgui(u8"DebugInfo", CImGuiManager::IMGUITYPE_DEFOULT);

	// FPS値の取得
	int fps = GetFPS();

	// FPS値
	ImGui::Text("FPS : %d", fps);

	ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

	// シード値
	ImGui::Text("SEED : %d", CGame::GetSeed());

	ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

	ImGui::Text("BG Color:");

	ImGui::ColorEdit4("col", m_bgCol);

	ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

	// ライトの個数
	ImGui::Text("Light Num : %d", CLight::GetLightNum());

	ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

	// プレイヤーのデバッグ情報の表示処理
	CGuiInfo::PlayerInfo();

	ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

	// 敵のデバッグ情報の表示処理
	CGuiInfo::EnemyInfo();

	ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

	// カメラのデバッグ情報の表示処理
	CGuiInfo::CameraInfo();

	ImGui::End();

#endif

}
//=============================================================================
// 描画処理
//=============================================================================
void CRenderer::Draw(int fps)
{
	// 画面クリア
	m_pD3DDevice->Clear(0, NULL,
		(D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL),
		m_bgCol, 1.0f, 0);

	// 描画開始
	if (SUCCEEDED(m_pD3DDevice->BeginScene()))
	{// 描画開始が成功した場合

		// すべてのオブジェクトの描画処理
		CObject::DrawAll();

		// フェードの取得
		CFade* pFade = CManager::GetFade();

		// フェードの描画処理
		if (pFade != nullptr)
		{
			// フェードの描画処理
			pFade->Draw();
		}

		// FPSのセット
		SetFPS(fps);
		
#ifdef _DEBUG

		// ImGuiの描画終了
		CImGuiManager::Instance().EndImguiRender();
#endif
		// 描画終了
		m_pD3DDevice->EndScene();
	}

	// バックバッファとフロントバッファの入れ替え
	m_pD3DDevice->Present(NULL, NULL, NULL, NULL);
}
//=============================================================================
// デバイスのリセット
//=============================================================================
void CRenderer::ResetDevice(void)
{
	if (!m_pD3DDevice || m_ResizeWidth == 0 || m_ResizeHeight == 0)
	{
		return;
	}

	// デバッグフォントの破棄
	m_pDebug->Uninit();

	// 3Dデバッグ表示の破棄
	m_pDebug3D->Uninit();

	//// 全UIにデバイス再設定通知
	//CUIManager* pUIManager = CUIManager::GetInstance();

	//for (auto ui : pUIManager->GetAllUI())
	//{
	//	ui->Uninit(); // CUIText の場合は D3DXFONT を再生成
	//}

	m_d3dpp.BackBufferWidth = m_ResizeWidth;
	m_d3dpp.BackBufferHeight = m_ResizeHeight;
	m_ResizeWidth = m_ResizeHeight = 0;

	ImGui_ImplDX9_InvalidateDeviceObjects();

	// サムネイルのリリース通知
	CManager::ReleaseThumbnail();

	HRESULT hr = m_pD3DDevice->Reset(&m_d3dpp);

	if (hr == D3DERR_INVALIDCALL)
	{
		IM_ASSERT(0);
	}

	ImGui_ImplDX9_CreateDeviceObjects();

	// サムネイルのリセット通知
	CManager::ResetThumbnail();

	// レンダーステートの設定
	m_pD3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pD3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pD3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// サンプラーステートの設定
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	m_pD3DDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	// テクスチャステージステートの設定
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT);

	// デバッグフォントの初期化
	m_pDebug->Init();

	// 3Dデバッグ表示の初期化
	m_pDebug3D->Init();

	//for (auto ui : pUIManager->GetAllUI())
	//{
	//	ui->Init(); // CUIText の場合は D3DXFONT を再生成
	//}

	// デバイスリセット通知(シーンライト)
	CManager::OnDeviceReset();
}
//=============================================================================
// 頂点シェーダコンパイル用関数
//=============================================================================
HRESULT CRenderer::CompileVertexShader(
	const char* filename,         // HLSLファイル名
	const char* entryPoint,       // エントリポイント名
	LPDIRECT3DVERTEXSHADER9* ppVS,// 作成したシェーダのポインタ
	LPD3DXCONSTANTTABLE* ppConsts // 定数テーブルを受け取る場合
)
{
	if (!m_pD3DDevice)
	{
		return E_FAIL;
	}

	LPD3DXBUFFER pCode = nullptr;
	LPD3DXBUFFER pErr = nullptr;

	HRESULT hr = D3DXCompileShaderFromFile(
		filename,
		nullptr,
		nullptr,
		entryPoint,
		"vs_3_0",
		D3DXSHADER_DEBUG,
		&pCode,
		&pErr,
		ppConsts
	);

	if (FAILED(hr))
	{
		if (pErr)
		{
			OutputDebugStringA((char*)pErr->GetBufferPointer());
			pErr->Release();
		}
		if (pCode)
		{
			pCode->Release();
		}

		return hr;
	}

	hr = m_pD3DDevice->CreateVertexShader(
		(DWORD*)pCode->GetBufferPointer(),
		ppVS
	);

	pCode->Release();
	if (pErr)
	{
		pErr->Release();
	}

	return hr;
}
//=============================================================================
// ピクセルシェーダコンパイル用関数
//=============================================================================
HRESULT CRenderer::CompilePixelShader(
	const char* filename,         // HLSLファイル名
	const char* entryPoint,       // エントリポイント名
	LPDIRECT3DPIXELSHADER9* ppPS, // 作成したシェーダのポインタ
	LPD3DXCONSTANTTABLE* ppConsts // 定数テーブルを受け取る場合
)
{
	if (!m_pD3DDevice)
	{
		return E_FAIL;
	}

	LPD3DXBUFFER pCode = nullptr;
	LPD3DXBUFFER pErr = nullptr;

	HRESULT hr = D3DXCompileShaderFromFile(
		filename,
		nullptr,
		nullptr,
		entryPoint,
		"ps_3_0",
		D3DXSHADER_DEBUG,
		&pCode,
		&pErr,
		ppConsts
	);

	if (FAILED(hr))
	{
		if (pErr)
		{
			OutputDebugStringA((char*)pErr->GetBufferPointer());
			pErr->Release();
		}
		if (pCode)
		{
			pCode->Release();
		}

		return hr;
	}

	hr = m_pD3DDevice->CreatePixelShader(
		(DWORD*)pCode->GetBufferPointer(),
		ppPS
	);

	pCode->Release();
	if (pErr)
	{
		pErr->Release();
	}

	return hr;
}
//=============================================================================
// サイズの再設定
//=============================================================================
void CRenderer::OnResize(UINT width, UINT height)
{
	m_ResizeWidth = width;
	m_ResizeHeight = height;
}
//=============================================================================
// デバイスのリセットが必要かどうか
//=============================================================================
bool CRenderer::NeedsReset(void) const
{
	return (m_ResizeWidth != 0 && m_ResizeHeight != 0);
}