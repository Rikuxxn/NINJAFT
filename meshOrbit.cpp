//=============================================================================
//
// メッシュの軌道処理 [meshOrbit.cpp]
// Author: RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "meshOrbit.h"
#include "manager.h"


//=============================================================================
// コンストラクタ
//=============================================================================
CMeshOrbit::CMeshOrbit(int nPriority) : CMeshBase(nPriority)
{
	// 値のクリア
	m_Top		= INIT_VEC3;	// 上の位置
	m_Bottom	= INIT_VEC3;	// 下の位置
	m_col		= INIT_XCOL;	// 色
}
//=============================================================================
// デストラクタ
//=============================================================================
CMeshOrbit::~CMeshOrbit()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CMeshOrbit* CMeshOrbit::Create(const D3DXVECTOR3 top, const D3DXVECTOR3 bottom, const D3DXCOLOR col, const int nSegH)
{
	// インスタンスを生成
	CMeshOrbit* pMesh = new CMeshOrbit;

	// nullptrなら飛ばす
	if (pMesh == nullptr)
	{
		return nullptr;
	}

	// 縦の分割数
	const int nSegV = 1;

	// 頂点の総数
	int nNumVtx = (nSegH + 1) * (nSegV + 1);

	// インデックスの総数
	int nNumIdx = nNumVtx;

	// ポリゴンの総数
	int nNumPolygon = nNumVtx - 2;

	// 分割数の設定
	pMesh->SetSegment(nSegH, nSegV);

	// 頂点の要素の設定
	pMesh->SetVtxElement(nNumVtx, nNumPolygon, nNumIdx);

	// 初期化失敗時
	if (FAILED(pMesh->Init()))
	{
		// 終了処理
		pMesh->Uninit();

		pMesh = nullptr;

		return nullptr;
	}

	// テクスチャ座(横)
	float fPosTexH = 1.0f / nSegH;

	// 頂点数分回す
	for (int nCnt = 0; nCnt < nNumVtx; nCnt++)
	{
		// アルファ値の設定
		float fAlpha = col.a * (1.0f - (float)nCnt / nNumVtx);

		// 頂点の設定
		pMesh->SetVtxPos(top, nCnt);

		// 法線、色、テクスチャの設定
		pMesh->SetNormal(D3DXVECTOR3(0.0f, 1.0f, 0.0f), nCnt);
		pMesh->SetVtxColor(D3DXCOLOR(col.r, col.g, col.b, fAlpha), nCnt);
		pMesh->SetTexture(D3DXVECTOR2(fPosTexH * nCnt, 1.0f), nCnt);

		// インデックスバッファの設定
		pMesh->SetIndexBuffer((WORD)nCnt, nCnt);
	}

	// 4個前から始める
	for (int nCnt = nNumVtx - 4; nCnt >= 0; nCnt -= 2)
	{
		// 前の頂点座標を入れる
		D3DXVECTOR3 OldVtx0 = pMesh->GetVtxPos(nCnt);
		D3DXVECTOR3 OldVtx1 = pMesh->GetVtxPos(nCnt + 1);

		// 頂点座標の設定
		pMesh->SetVtxPos(OldVtx0, nCnt + 2);
		pMesh->SetVtxPos(OldVtx1, nCnt + 3);
	}

	// 新しい頂点を追加（先頭に入れる）
	pMesh->SetVtxPos(bottom, 0);
	pMesh->SetVtxPos(top, 1);

	// 設定処理
	pMesh->m_Bottom = bottom;
	pMesh->m_Top = top;
	pMesh->m_col = col;

	return pMesh;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CMeshOrbit::Init(void)
{
	// メッシュベースの初期化処理
	CMeshBase::Init();

	// テクスチャの設定
	SetTexPath("data/TEXTURE/.jpg");

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CMeshOrbit::Uninit(void)
{
	// メッシュベースの終了処理
	CMeshBase::Uninit();
}
//=============================================================================
// 更新処理
//=============================================================================
void CMeshOrbit::Update(void)
{
	int nSegH = GetSegH(); 	// 横の分割数の取得
	int nSegV = 1;			// 縦の分割数

	// 頂点の総数
	int nNumVtx = (nSegH + 1) * (nSegV + 1);

	// 4個前から始める
	for (int nCnt = nNumVtx - 4; nCnt >= 0; nCnt -= 2)
	{
		// 前の頂点座標を入れる
		D3DXVECTOR3 OldVtx0 = GetVtxPos(nCnt);
		D3DXVECTOR3 OldVtx1 = GetVtxPos(nCnt + 1);

		// 頂点座標の設定
		SetVtxPos(OldVtx0, nCnt + 2);
		SetVtxPos(OldVtx1, nCnt + 3);
	}

	// 新しい頂点を追加（先頭に入れる）
	SetVtxPos(m_Bottom, 0);
	SetVtxPos(m_Top, 1);
}
//=============================================================================
// 描画処理
//=============================================================================
void CMeshOrbit::Draw(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// カリング設定を無効化
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	// aブレンディング
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	// メッシュベースの描画処理
	CMeshBase::Draw();

	// aブレンディングをもとに戻す
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	// カリング設定を有効化
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}
