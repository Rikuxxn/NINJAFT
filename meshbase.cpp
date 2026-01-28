//=============================================================================
//
// メッシュのベース処理 [meshbase.cpp]
// Author: RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "meshbase.h"
#include "manager.h"


//=============================================================================
// コンストラクタ
//=============================================================================
CMeshBase::CMeshBase(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	m_pVtxBuff		= nullptr;		// 頂点バッファへのポインタ
	m_pIdxBuff		= nullptr;		// インデックスバッファへのポインタ
	m_pos			= INIT_VEC3;	// 位置
	m_rot			= INIT_VEC3;	// 向き
	m_mtxWorld		= {};			// ワールドマトリックス
	m_nSegmentH		= 0;			// 横の分割数
	m_nSegmentV		= 0;			// 縦の分割数
	m_nIdxTexture	= -1;			// テクスチャのインデックス
	m_nNumVtx		= 0;			// 頂点数
	m_nNumPolygon	= 0;			// ポリゴン数
	m_nNumIdx		= 0;			// インデックスバッファ
}
//=============================================================================
// デストラクタ
//=============================================================================
CMeshBase::~CMeshBase()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CMeshBase::Init(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// 頂点バッファの生成
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * m_nNumVtx,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_3D,
		D3DPOOL_MANAGED,
		&m_pVtxBuff,
		NULL);

	// インデックスバッファの生成
	pDevice->CreateIndexBuffer(sizeof(WORD) * m_nNumIdx,
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&m_pIdxBuff,
		NULL);

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CMeshBase::Uninit(void)
{
	// 頂点バッファの破棄
	if (m_pVtxBuff != nullptr)
	{
		m_pVtxBuff->Release();
		m_pVtxBuff = nullptr;
	}

	// インデックスバッファの破棄
	if (m_pIdxBuff != nullptr)
	{
		m_pIdxBuff->Release();
		m_pIdxBuff = nullptr;
	}

	// 自身の破棄
	this->Release();
}
//=============================================================================
// 更新処理
//=============================================================================
void CMeshBase::Draw(void)
{
	// デバイスのポインタ
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// テクスチャの取得
	CTexture* pTexture = CManager::GetTexture();

	// 計算用のマトリックスを宣言
	D3DXMATRIX mtxRot, mtxTrans;

	// ワールドマトリックスの初期化
	D3DXMatrixIdentity(&m_mtxWorld);

	// 向きを反映
	D3DXMatrixRotationYawPitchRoll(&mtxRot, m_rot.y, m_rot.x, m_rot.z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxRot);

	// 位置を反映
	D3DXMatrixTranslation(&mtxTrans, m_pos.x, m_pos.y, m_pos.z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxTrans);

	// ワールドマトリックスの設定
	pDevice->SetTransform(D3DTS_WORLD, &m_mtxWorld);

	// 頂点バッファをデバイスのデータストリームに設定
	pDevice->SetStreamSource(0, m_pVtxBuff, 0, sizeof(VERTEX_3D));

	// インデックスバッファをデータストリームに設定
	pDevice->SetIndices(m_pIdxBuff);

	// テクスチャフォーマットの設定
	pDevice->SetFVF(FVF_VERTEX_3D);

	// テクスチャ割り当て
	pDevice->SetTexture(0, pTexture->GetAddress(m_nIdxTexture));

	// ポリゴンの描画
	pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, m_nNumVtx, 0, m_nNumPolygon);
}
//=============================================================================
// 頂点座標の設定
//=============================================================================
void CMeshBase::SetVtxBuffer(const D3DXVECTOR3 pos, const int nIdx, const D3DXVECTOR2 tex, const D3DXVECTOR3 nor, const D3DXCOLOR col)
{
	VERTEX_3D* pVtx = nullptr;

	// 頂点バッファをロック
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	pVtx[nIdx].pos = pos;

	//法線ベクトルの設定
	pVtx[nIdx].nor = nor;

	//頂点カラーの設定
	pVtx[nIdx].col = col;

	//テクスチャ座標の設定
	pVtx[nIdx].tex = tex;

	// 頂点バッファをアンロック
	m_pVtxBuff->Unlock();
}
//=============================================================================
// インデックスバッファの設定
//=============================================================================
void CMeshBase::SetIndexBuffer(const WORD Idx, const int nCnt)
{
	WORD* pIdx;

	//インデックスバッファのロック
	m_pIdxBuff->Lock(0, 0, (void**)&pIdx, 0);

	pIdx[nCnt] = Idx;

	//インデックスバッファのアンロック
	m_pIdxBuff->Unlock();
}
//=============================================================================
// テクスチャの設定処理
//=============================================================================
void CMeshBase::SetTexPath(const char* pTextureName)
{
	// テクスチャの取得
	CTexture* pTexture = CManager::GetTexture();

	if (pTextureName == NULL)
	{
		m_nIdxTexture = -1;
		return;
	}

	// テクスチャのIDの設定
	m_nIdxTexture = pTexture->RegisterDynamic(pTextureName);
}
//=============================================================================
// 頂点の要素の設定処理
//=============================================================================
void CMeshBase::SetVtxElement(const int vertex, const int polygon, const int index)
{
	// 頂点数の設定
	m_nNumVtx = vertex;

	// ポリゴン数の設定
	m_nNumPolygon = polygon;

	// インデックス数の設定
	m_nNumIdx = index;
}
//=============================================================================
// マトリックスの設定
//=============================================================================
void CMeshBase::SetMatrix(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	//計算用のマトリックス
	D3DXMATRIX mtxRot, mtxTrans;

	//ワールドマトリックスの初期化
	D3DXMatrixIdentity(&m_mtxWorld);

	//向きを反映
	D3DXMatrixRotationYawPitchRoll(&mtxRot, m_rot.y, m_rot.x, m_rot.z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxRot);

	//位置を反映
	D3DXMatrixTranslation(&mtxTrans, m_pos.x, m_pos.y, m_pos.z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxTrans);

	//ワールドマトリックスの設定
	pDevice->SetTransform(D3DTS_WORLD, &m_mtxWorld);
}
//=============================================================================
// 頂点の位置の設定
//=============================================================================
void CMeshBase::SetVtxPos(const D3DXVECTOR3 pos, const int nIdx)
{
	// 頂点情報のポインタを宣言
	VERTEX_3D* pVtx = nullptr;

	// nullptrだったら
	if (m_pVtxBuff == nullptr)
	{
		return;
	}

	// 頂点バッファをロック
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	pVtx[nIdx].pos = pos;

	// 頂点バッファをアンロック
	m_pVtxBuff->Unlock();
}
//=============================================================================
// 法線の設定
//=============================================================================
void CMeshBase::SetNormal(const D3DXVECTOR3 nor, const int nIdx)
{
	// 頂点情報のポインタを宣言
	VERTEX_3D* pVtx = nullptr;

	// nullptrだったら
	if (m_pVtxBuff == nullptr)
	{
		return;
	}

	// 頂点バッファをロック
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	pVtx[nIdx].nor = nor;

	// 頂点バッファをアンロック
	m_pVtxBuff->Unlock();
}
//=============================================================================
// 色の設定
//=============================================================================
void CMeshBase::SetVtxColor(const D3DXCOLOR col, const int nIdx)
{
	// 頂点情報のポインタを宣言
	VERTEX_3D* pVtx = nullptr;

	// nullptrだったら
	if (m_pVtxBuff == nullptr)
	{
		return;
	}

	// 頂点バッファをロック
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	pVtx[nIdx].col = col;

	// 頂点バッファをアンロック
	m_pVtxBuff->Unlock();
}
//=============================================================================
// テクスチャバッファの設定
//=============================================================================
void CMeshBase::SetTexture(const D3DXVECTOR2 tex, const int nIdx)
{
	// 頂点情報のポインタを宣言
	VERTEX_3D* pVtx = nullptr;

	// nullptrだったら
	if (m_pVtxBuff == nullptr)
	{
		return;
	}

	// 頂点バッファをロック
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	pVtx[nIdx].tex = tex;

	// 頂点バッファをアンロック
	m_pVtxBuff->Unlock();
}
//=============================================================================
// 頂点座標の取得
//=============================================================================
D3DXVECTOR3 CMeshBase::GetVtxPos(const int nIdx)
{
	// 頂点情報のポインタを宣言
	VERTEX_3D* pVtx = nullptr;

	D3DXVECTOR3 out;

	// nullptrだったら
	if (m_pVtxBuff == nullptr)
	{
		return INIT_VEC3;
	}

	// 頂点バッファをロック
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	out = pVtx[nIdx].pos;

	// 頂点バッファをアンロック
	m_pVtxBuff->Unlock();

	return out;
}
//=============================================================================
// インデックス番号の取得
//=============================================================================
int CMeshBase::GetIndex(const int nIdx)
{
	WORD* pIdx;

	int out = 0;

	// nullだったら
	if (m_pIdxBuff == nullptr)
	{
		return 0;
	}

	//インデックスバッファのロック
	m_pIdxBuff->Lock(0, 0, (void**)&pIdx, 0);

	out = pIdx[nIdx];

	//インデックスバッファのアンロック
	m_pIdxBuff->Unlock();

	return out;
}
//=============================================================================
// 色の取得
//=============================================================================
D3DXCOLOR CMeshBase::GetColor(const int nIdx)
{
	// 頂点情報のポインタを宣言
	VERTEX_3D* pVtx = nullptr;

	D3DXCOLOR out;

	// nullptrだったら
	if (m_pVtxBuff == nullptr)
	{
		return INIT_XCOL_WHITE;
	}

	// 頂点バッファをロック
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	out = pVtx[nIdx].col;

	// 頂点バッファをアンロック
	m_pVtxBuff->Unlock();

	return out;
}
//=============================================================================
// 法線の正規化処理
//=============================================================================
D3DXVECTOR3 CMeshBase::NormalizeNormal(const int nIdx)
{
	// 頂点情報のポインタを宣言
	VERTEX_3D* pVtx = nullptr;

	// nullptrだったら
	if (m_pVtxBuff == nullptr)
	{
		return INIT_VEC3;
	}

	// 頂点バッファをロック
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	// 法線
	D3DXVECTOR3 nor = pVtx[nIdx].pos - m_pos;

	// 正規化する
	D3DXVec3Normalize(&nor, &nor);

	// 頂点バッファをアンロック
	m_pVtxBuff->Unlock();

	return nor;
}
