//=============================================================================
//
// 項目処理 [item.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "item.h"
#include "renderer.h"
#include "manager.h"
#include "itemselect.h"


//=============================================================================
// コンストラクタ
//=============================================================================
CItem::CItem(int nPriority) : CObject2D(nPriority)
{
	// 値のクリア
	memset(m_szPath, 0, sizeof(m_szPath));
	m_nIdxTexture = 0;
	m_isSelected = false;
}
//=============================================================================
// デストラクタ
//=============================================================================
CItem::~CItem()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CItem* CItem::Create(ITEM type, D3DXVECTOR3 pos, float fWidth, float fHeight)
{
	CItem* pItem = nullptr;

	switch (type)
	{
	case ITEM_ID_PLAY:
		pItem = new CPlay;
		pItem->SetPath("data/TEXTURE/ui_start.png");
		break;
	case ITEM_ID_EXIT:
		pItem = new CExit;
		pItem->SetPath("data/TEXTURE/ui_exit.png");
		break;
	default:
		pItem = new CItem;
		break;
	}

	// nullptrだったら
	if (pItem == nullptr)
	{
		return nullptr;
	}

	pItem->SetPos(pos);
	pItem->SetSize(fWidth, fHeight);
	pItem->SetCol(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));

	// 初期化失敗時
	if (FAILED(pItem->Init()))
	{
		return nullptr;
	}

	return pItem;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CItem::Init(void)
{
	// テクスチャの登録
	m_nIdxTexture = CManager::GetTexture()->RegisterDynamic(m_szPath);

	// 2Dオブジェクトの初期化処理
	CObject2D::Init();

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CItem::Uninit(void)
{
	// 2Dオブジェクトの終了処理
	CObject2D::Uninit();
}
//=============================================================================
// 更新処理
//=============================================================================
void CItem::Update(void)
{
	// 選択項目の色の切り替え
	if (IsMouseOver() || m_isSelected)
	{
		SetCol(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
	}
	else
	{
		SetCol(D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.5f));
	}

	// 2Dオブジェクトの更新処理
	CObject2D::Update();
}
//=============================================================================
// 描画処理
//=============================================================================
void CItem::Draw(void)
{
	// テクスチャの取得
	CTexture* pTexture = CManager::GetTexture();

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// テクスチャの設定
	pDevice->SetTexture(0, pTexture->GetAddress(m_nIdxTexture));

	// 2Dオブジェクトの描画処理
	CObject2D::Draw();
}
//=============================================================================
// マウスカーソルの判定処理
//=============================================================================
bool CItem::IsMouseOver(void)
{
	// マウスカーソルの位置を取得
	POINT cursorPos;
	GetCursorPos(&cursorPos);

	// ウィンドウハンドルを取得
	HWND hwnd = GetActiveWindow();

	// スクリーン座標をクライアント座標に変換
	ScreenToClient(hwnd, &cursorPos);

	// クライアントサイズを取得
	RECT clientRect;
	GetClientRect(hwnd, &clientRect);

	float left = GetPos().x - GetWidth();
	float right = GetPos().x + GetWidth();
	float top = GetPos().y - GetHeight();
	float bottom = GetPos().y + GetHeight();

	return (cursorPos.x >= left && cursorPos.x <= right &&
		cursorPos.y >= top && cursorPos.y <= bottom);
}
//=============================================================================
// ステージIDを返す処理
//=============================================================================
CItem::ITEM CItem::GetItemId(void) const
{
	if (dynamic_cast<const CPlay*>(this)) return ITEM_ID_PLAY;
	if (dynamic_cast<const CExit*>(this)) return ITEM_ID_EXIT;

	return ITEM_MAX; // 想定外
}


//=============================================================================
// プレイ項目のコンストラクタ
//=============================================================================
CPlay::CPlay()
{
	// 値のクリア

}
//=============================================================================
// プレイ項目のデストラクタ
//=============================================================================
CPlay::~CPlay()
{
	// なし
}
//=============================================================================
// 選択時の処理
//=============================================================================
void CPlay::Execute(void)
{
	// ゲーム画面に移行
	CManager::GetFade()->SetFade(CScene::MODE_TUTORIAL);
}


//=============================================================================
// やめる項目のコンストラクタ
//=============================================================================
CExit::CExit()
{
	// 値のクリア

}
//=============================================================================
// やめる項目のデストラクタ
//=============================================================================
CExit::~CExit()
{
	// なし
}
//=============================================================================
// 選択時の処理
//=============================================================================
void CExit::Execute(void)
{
	//ウィンドウを破棄する
	PostQuitMessage(0);
}
