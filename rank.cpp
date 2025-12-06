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
	m_pVtxBuff = nullptr;					// 頂点バッファ
	m_pos = D3DXVECTOR3(0.0f,0.0f,0.0f);	// 位置
	m_fWidth = 0.0f;						// 幅
	m_fHeight = 0.0f;						// 高さ
	m_nIdxTexture = 0;
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
	CRank* pRank;

	pRank = new CRank;

	pRank->m_pos = pos;
	pRank->m_fWidth = fWidth;
	pRank->m_fHeight = fHeight;
	pRank->m_fRank = fRank;

	// 初期化処理
	pRank->Init();

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
	pVtx[0].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVtx[1].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVtx[2].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVtx[3].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

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