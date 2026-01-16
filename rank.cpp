//=============================================================================
//
// ランキング順位表示処理 [rank.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "rank.h"
#include "renderer.h"
#include "manager.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CRank::CRank(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	m_pVtxBuff		= nullptr;		// 頂点バッファ
	m_pos			= INIT_VEC3;	// 位置
	m_fWidth		= 0.0f;			// 幅
	m_fHeight		= 0.0f;			// 高さ
	m_nIdxTexture	= 0;			// テクスチャインデックス
}
//=============================================================================
// デストラクタ
//=============================================================================
CRank::~CRank()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CRank* CRank::Create(D3DXVECTOR3 pos, float fWidth, float fHeight, float fRank)
{
	CRank* pRank = new CRank;

	// nullptrだったら
	if (pRank == nullptr)
	{
		return nullptr;
	}

	pRank->m_pos = pos;
	pRank->m_fWidth = fWidth;
	pRank->m_fHeight = fHeight;
	pRank->m_fRank = fRank;

	// 初期化失敗時
	if (FAILED(pRank->Init()))
	{
		return nullptr;
	}

	return pRank;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CRank::Init(void)
{
	// デバイス取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	m_nIdxTexture = CManager::GetTexture()->RegisterDynamic("data/TEXTURE/rank.png");

	// 頂点バッファの生成
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&m_pVtxBuff,
		NULL);

	VERTEX_2D* pVtx;// 頂点情報へのポインタ

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	// 頂点座標の設定
	pVtx[0].pos = D3DXVECTOR3(m_pos.x, m_pos.y, 0.0f);
	pVtx[1].pos = D3DXVECTOR3(m_pos.x + m_fWidth, m_pos.y, 0.0f);
	pVtx[2].pos = D3DXVECTOR3(m_pos.x, m_pos.y + m_fHeight, 0.0f);
	pVtx[3].pos = D3DXVECTOR3(m_pos.x + m_fWidth, m_pos.y + m_fHeight, 0.0f);

	// rhwの設定
	pVtx[0].rhw = 1.0f;
	pVtx[1].rhw = 1.0f;
	pVtx[2].rhw = 1.0f;
	pVtx[3].rhw = 1.0f;

	// 頂点カラーの設定
	if (m_fRank == RANK_FIRST)
	{// 1位
		pVtx[0].col = D3DCOLOR_RGBA(255, 255, 255, 255);
		pVtx[1].col = D3DCOLOR_RGBA(255, 215, 0, 255);
		pVtx[2].col = D3DCOLOR_RGBA(255, 215, 0, 255);
		pVtx[3].col = D3DCOLOR_RGBA(255, 255, 255, 255);
	}
	else if (m_fRank == RANK_SECOND)
	{// 2位
		pVtx[0].col = D3DCOLOR_RGBA(255, 255, 255, 255);
		pVtx[1].col = D3DCOLOR_RGBA(192, 192, 192, 255);
		pVtx[2].col = D3DCOLOR_RGBA(192, 192, 192, 255);
		pVtx[3].col = D3DCOLOR_RGBA(255, 255, 255, 255);
	}
	else if (m_fRank == RANK_THIRD)
	{// 3位
		pVtx[0].col = D3DCOLOR_RGBA(255, 255, 255, 255);
		pVtx[1].col = D3DCOLOR_RGBA(196, 112, 34, 255);
		pVtx[2].col = D3DCOLOR_RGBA(196, 112, 34, 255);
		pVtx[3].col = D3DCOLOR_RGBA(255, 255, 255, 255);
	}
	else
	{// その他
		pVtx[0].col = D3DCOLOR_RGBA(255, 255, 255, 255);
		pVtx[1].col = D3DCOLOR_RGBA(255, 255, 255, 255);
		pVtx[2].col = D3DCOLOR_RGBA(255, 255, 255, 255);
		pVtx[3].col = D3DCOLOR_RGBA(255, 255, 255, 255);
	}

	// テクスチャ座標の設定
	pVtx[0].tex = D3DXVECTOR2(m_fRank * 0.2f, 0.0f);
	pVtx[1].tex = D3DXVECTOR2(m_fRank * 0.2f + 0.2f, 0.0f);
	pVtx[2].tex = D3DXVECTOR2(m_fRank * 0.2f, 1.0f);
	pVtx[3].tex = D3DXVECTOR2(m_fRank * 0.2f + 0.2f, 1.0f);

	// 頂点バッファをアンロックする
	m_pVtxBuff->Unlock();

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CRank::Uninit(void)
{
	// 頂点バッファの破棄
	if (m_pVtxBuff != nullptr)
	{
		m_pVtxBuff->Release();
		m_pVtxBuff = nullptr;
	}

	this->Release();
}
//=============================================================================
// 更新処理
//=============================================================================
void CRank::Update(void)
{
	VERTEX_2D* pVtx;// 頂点情報へのポインタ

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	// 座標
	pVtx[0].pos = { m_pos.x,             m_pos.y,              0.0f };
	pVtx[1].pos = { m_pos.x + m_fWidth,  m_pos.y,              0.0f };
	pVtx[2].pos = { m_pos.x,             m_pos.y + m_fHeight,  0.0f };
	pVtx[3].pos = { m_pos.x + m_fWidth,  m_pos.y + m_fHeight,  0.0f };

	// 頂点バッファをアンロックする
	m_pVtxBuff->Unlock();
}
//=============================================================================
// 描画処理
//=============================================================================
void CRank::Draw(void)
{
	if (CManager::GetMode() == CScene::MODE_RANKING)
	{
		// テクスチャの取得
		CTexture* pTexture = CManager::GetTexture();

		// デバイスの取得
		LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

		// 頂点バッファをデータストリームに設定
		pDevice->SetStreamSource(0, m_pVtxBuff, 0, sizeof(VERTEX_2D));

		// 頂点フォーマットの設定
		pDevice->SetFVF(FVF_VERTEX_2D);

		// テクスチャの設定
		pDevice->SetTexture(0, pTexture->GetAddress(m_nIdxTexture));

		// ポリゴンの描画
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
	}
}