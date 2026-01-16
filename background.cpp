//=============================================================================
//
// 背景処理 [background.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "background.h"
#include "manager.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CBackground::CBackground()
{
	// 値のクリア
	m_nIdxTexture = 0;						// テクスチャインデックス
	memset(m_szPath, 0, sizeof(m_szPath));	// ファイルパス
}
//=============================================================================
// デストラクタ
//=============================================================================
CBackground::~CBackground()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CBackground* CBackground::Create(const char* path, float anchorX, float anchorY, float widthRate, float heightRate)
{
	CBackground* pBackground = new CBackground();

	// nullptrだったら
	if (pBackground == nullptr)
	{
		return nullptr;
	}

	pBackground->SetPath(path);
	pBackground->SetAnchor(anchorX, anchorY);
	pBackground->SetCol(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
	pBackground->SetSizeRate(widthRate, heightRate);

	// 初期化失敗時
	if (FAILED(pBackground->Init()))
	{
		return nullptr;
	}

	return pBackground;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CBackground::Init(void)
{
	// テクスチャの取得
	m_nIdxTexture = CManager::GetTexture()->RegisterDynamic(m_szPath);

	// 2Dオブジェクトの初期化処理
	CObject2D::Init();

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CBackground::Uninit(void)
{
	// 2Dオブジェクトの終了処理
	CObject2D::Uninit();
}
//=============================================================================
// 更新処理
//=============================================================================
void CBackground::Update(void)
{
	// バックバッファサイズの取得
	float sw = (float)CManager::GetRenderer()->GetBackBufferWidth();
	float sh = (float)CManager::GetRenderer()->GetBackBufferHeight();

	float w = sw * m_layout.widthRate;
	float h = sh * m_layout.heightRate;

	float x = sw * m_layout.anchorX - w * 0.5f;
	float y = sh * m_layout.anchorY - h * 0.5f;

	// サイズの更新
	SetSize(w, h);

	// 位置の更新
	SetPos({ x, y, 0.0f });

	// 2Dオブジェクトの更新処理
	CObject2D::Update();
}
//=============================================================================
// 描画処理
//=============================================================================
void CBackground::Draw(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// テクスチャの設定
	pDevice->SetTexture(0, CManager::GetTexture()->GetAddress(m_nIdxTexture));

	// 2Dオブジェクトの描画処理
	CObject2D::Draw();
}
