//=============================================================================
//
// メッシュドーム処理 [meshdome.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "meshdome.h"
#include "manager.h"


//=============================================================================
// コンストラクタ
//=============================================================================
CMeshDome::CMeshDome(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	m_nIdxTexture = 0;
}
//=============================================================================
// デストラクタ
//=============================================================================
CMeshDome::~CMeshDome()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CMeshDome* CMeshDome::Create(D3DXVECTOR3 pos, int nRadius)
{
	// インスタンス生成
	CMeshDome* pDome = new CMeshDome;

	// nullptrだったら
	if (pDome == nullptr)
	{
		return nullptr;
	}

	pDome->m_pos = pos;
	pDome->m_nRadius = nRadius;

	// 初期化失敗時
	if (FAILED(pDome->Init()))
	{
		return nullptr;
	}

	return nullptr;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CMeshDome::Init(void)
{
	// テクスチャの登録
	m_nIdxTexture = CManager::GetTexture()->RegisterDynamic("data/TEXTURE/sky.jpg");

	// デバイスのポインタ
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// 頂点バッファの生成
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * MESHDOME_VERTEX,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_3D,
		D3DPOOL_MANAGED,
		&m_pVtxBuff,
		NULL);

	// インデックスバッファの生成
	pDevice->CreateIndexBuffer(sizeof(WORD) * MESHDOME_INDEX,
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&m_pIdxBuff,
		NULL);

	// 頂点バッファのポインタ
	VERTEX_3D* pVtx = nullptr;

	// 頂点バッファをロック
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntV = 0; nCntV <= MESHDOME_Z; nCntV++)
	{
		for (int nCntH = 0; nCntH <= MESHDOME_X; nCntH++)
		{
			// nCntH番目の角度を計算
			float angleH = (360.0f / MESHDOME_X) * nCntH;

			// nCntV番目の角度を計算
			float angleV = 90.0f * ((float)nCntV / MESHDOME_Z);

			// 頂点の位置を設定
			pVtx[nCntV * (MESHDOME_X + 1) + nCntH].pos = D3DXVECTOR3(
				sinf(D3DXToRadian(angleH)) * m_nRadius * cosf(D3DXToRadian(angleV)), // X座標
				m_nRadius * sinf(D3DXToRadian(angleV)),								 // Y座標
				cosf(D3DXToRadian(angleH)) * m_nRadius * cosf(D3DXToRadian(angleV))  // Z座標
			);

			// 法線を計算 (頂点座標からの単位ベクトル)
			D3DXVECTOR3 normal = D3DXVECTOR3(
				pVtx[nCntV * (MESHDOME_X + 1) + nCntH].pos.x / m_nRadius,
				pVtx[nCntV * (MESHDOME_X + 1) + nCntH].pos.y / m_nRadius,
				pVtx[nCntV * (MESHDOME_X + 1) + nCntH].pos.z / m_nRadius
			);

			// 正規化する
			D3DXVec3Normalize(&normal, &normal);

			// 法線に割り当て
			pVtx[nCntV * (MESHDOME_X + 1) + nCntH].nor = normal;

			// 頂点カラーを設定
			pVtx[nCntV * (MESHDOME_X + 1) + nCntH].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

			// テクスチャ座標を設定
			pVtx[nCntV * (MESHDOME_X + 1) + nCntH].tex = D3DXVECTOR2(
				(float)nCntH / MESHDOME_X,						// U座標（円周方向）
				1.0f - (float)nCntV / MESHDOME_Z				// V座標（高さ方向）
			);
		}
	}

	// 頂点バッファをアンロック
	m_pVtxBuff->Unlock();

	// インデックスバッファのポインタ
	WORD* pIdx = nullptr;

	// インデックスバッファをロック
	m_pIdxBuff->Lock(0, 0, (void**)&pIdx, 0);

	// インデックスの設定
	WORD idx = 0;

	for (int nCntV = 0; nCntV < MESHDOME_Z; nCntV++)
	{
		for (int nCntX = 0; nCntX < MESHDOME_X; nCntX++)
		{
			// 基準のインデックス
			WORD baseIndex = nCntV * (MESHDOME_X + 1) + nCntX;

			// 三角形1
			pIdx[idx++] = baseIndex;
			pIdx[idx++] = baseIndex + MESHDOME_X + 1;
			pIdx[idx++] = baseIndex + 1;

			// 三角形2
			pIdx[idx++] = baseIndex + 1;
			pIdx[idx++] = baseIndex + MESHDOME_X + 1;
			pIdx[idx++] = baseIndex + MESHDOME_X + 2;
		}

	}

	// インデックスバッファをアンロック
	m_pIdxBuff->Unlock();

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CMeshDome::Uninit(void)
{
	// インデックスバッファの解放
	if (m_pIdxBuff != nullptr)
	{
		m_pIdxBuff->Release();
		m_pIdxBuff = nullptr;
	}

	// 頂点バッファの破棄
	if (m_pVtxBuff != nullptr)
	{
		m_pVtxBuff->Release();
		m_pVtxBuff = nullptr;
	}

	// オブジェクトの破棄(自分自身)
	this->Release();
}
//=============================================================================
// 更新処理
//=============================================================================
void CMeshDome::Update(void)
{
}
//=============================================================================
// 描画処理
//=============================================================================
void CMeshDome::Draw(void)
{
	// テクスチャの取得
	CTexture* pTexture = CManager::GetTexture();

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// 計算用マトリックス
	D3DXMATRIX mtxRot, mtxTrans;

	// ワールドマトリックスの初期化
	D3DXMatrixIdentity(&m_mtxWorld);

	// 向きを反映
	D3DXMatrixRotationYawPitchRoll(&mtxRot, m_rot.y, m_rot.x, m_rot.z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxRot);

	// 位置を反映
	D3DXMatrixTranslation(&mtxTrans, m_pos.x, m_pos.y, m_pos.z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxTrans);

	// ワールドマトリックスを設定
	pDevice->SetTransform(D3DTS_WORLD, &m_mtxWorld);

	// 頂点バッファをデータストリームに設定
	pDevice->SetStreamSource(0, m_pVtxBuff, 0, sizeof(VERTEX_3D));

	// インデックスバッファをデータストリームに設定
	pDevice->SetIndices(m_pIdxBuff);

	// 頂点フォーマットの設定
	pDevice->SetFVF(FVF_VERTEX_3D);

	// テクスチャの設定
	pDevice->SetTexture(0, pTexture->GetAddress(m_nIdxTexture));

	// ポリゴンの描画
	pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, MESHDOME_VERTEX, 0, MESHDOME_INDEX);
}
