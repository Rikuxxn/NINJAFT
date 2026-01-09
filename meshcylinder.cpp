//=============================================================================
//
// メッシュシリンダー処理 [meshcylinder.h]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "meshcylinder.h"
#include "manager.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CMeshCylinder::CMeshCylinder(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	m_pVtxBuff		= nullptr;	// 頂点バッファへのポインタ
	m_pIdxBuff		= nullptr;	// インデックスバッファへのポインタ
	m_mtxWorld		= {};		// ワールドマトリックス
	m_pos			= INIT_VEC3;// 位置
	m_rot			= INIT_VEC3;// 向き
	m_col			= INIT_XCOL;// 色
	m_fRadius		= 0.0f;		// 半径
	m_incRadius		= 0.0f;		// 半径の増加量
	m_nLife			= 0;		// 現在の寿命
	m_nMaxLife		= 0;		// 設定時の寿命
	m_decAlpha		= 0.0f;		// アルファ値の減少量
	m_fLineweight	= 0.0f;		// 太さ
}
//=============================================================================
// デストラクタ
//=============================================================================
CMeshCylinder::~CMeshCylinder()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CMeshCylinder* CMeshCylinder::Create(D3DXVECTOR3 pos, D3DXCOLOR col, float fRad, float fWeight, float incRad, int nLife, float decAlpha)
{
	CMeshCylinder* pCylinder = new CMeshCylinder;

	// nullptrだったら
	if (pCylinder == nullptr)
	{
		return nullptr;
	}

	pCylinder->SetPos(pos);
	pCylinder->SetCol(col);
	pCylinder->SetRadius(fRad);
	pCylinder->SetLineweight(fWeight);
	pCylinder->SetIncRadius(incRad);
	pCylinder->SetLife(nLife);
	pCylinder->SetDecAlpha(decAlpha);

	// 初期化失敗時
	if (FAILED(pCylinder->Init()))
	{
		return nullptr;
	}

	return pCylinder;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CMeshCylinder::Init(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// 頂点バッファの生成
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * MESHCYLINDER_VERTEX,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_3D,
		D3DPOOL_MANAGED,
		&m_pVtxBuff,
		NULL);

	// インデックスバッファの生成
	pDevice->CreateIndexBuffer(sizeof(WORD) * MESHCYLINDER_INDEX,
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&m_pIdxBuff,
		NULL);

	VERTEX_3D* pVtx = nullptr;// 頂点情報へのポインタ

	int nCnt = 0;
	//float tex = 1.0f / MESHCYLINDER_X;
	//float tex2 = 1.0f / MESHCYLINDER_Z;
	D3DXVECTOR3 nor;

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntV = 0; nCntV < MESHCYLINDER_V + 1; nCntV++)
	{
		for (int nCntH = 0; nCntH < MESHCYLINDER_H + 1; nCntH++)
		{
			float Angle = (D3DX_PI * 2.0f) / MESHCYLINDER_H * nCntH;

			pVtx[nCnt].pos = D3DXVECTOR3(
				sinf(Angle) * m_fRadius - sinf(Angle),
				0.0f, 
				cosf(Angle) * m_fRadius - cosf(Angle)
			);

			nor.x = pVtx[nCnt].pos.x - 0.0f;
			nor.y = pVtx[nCnt].pos.y - 0.0f;
			nor.z = pVtx[nCnt].pos.z - 0.0f;

			// 法線の正規化
			D3DXVec3Normalize(&pVtx[nCnt].nor, &nor);

			// 頂点カラーの設定
			pVtx[nCnt].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

			// テクスチャ座標の設定
			pVtx[nCnt].tex = D3DXVECTOR2(/*tex * */(float)nCntV, /*tex2 * */(float)nCntH);

			nCnt++;
		}
	}

	// 頂点バッファをアンロックする
	m_pVtxBuff->Unlock();

	WORD* pIdx;// インデックス情報へのポインタ

	// インデックスバッファをロックし、頂点情報へのポインタを取得
	m_pIdxBuff->Lock(0, 0, (void**)&pIdx, 0);

	int nCntIdx3 = MESHCYLINDER_X + 1;
	int Num = 0;
	int nCntNum = 0;

	// インデックスの設定
	for (int nCntIdx = 0; nCntIdx < MESHCYLINDER_Z; nCntIdx++)
	{
		for (int nCntIdx2 = 0; nCntIdx2 <= MESHCYLINDER_X; nCntIdx2++, nCntIdx3++, Num++)
		{
			pIdx[nCntNum] = (WORD)nCntIdx3;

			pIdx[nCntNum + 1] = (WORD)Num;

			nCntNum += 2;
		}

		// 最後の行かどうか
		// NOTE: 最後の行に縮退ポリゴンが無い
		if (nCntIdx != MESHCYLINDER_Z - 1)
		{
			pIdx[nCntNum] = (WORD)Num - 1;

			pIdx[nCntNum + 1] = (WORD)nCntIdx3;

			nCntNum += 2;
		}
	}

	// インデックスバッファをアンロックする
	m_pIdxBuff->Unlock();

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CMeshCylinder::Uninit(void)
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
void CMeshCylinder::Update(void)
{
	m_col.a -= m_decAlpha;// アルファ値を徐々に下げる

	m_nLife--;

	float fMugLife = (float)m_nLife / 50;
	float fAlpha = m_col.a * fMugLife;

	// 寿命またはアルファ値が0になったら
	if (m_nLife <= 0)
	{
		// 終了
		Uninit();
		return;
	}

	// 半径を徐々に広げる
	m_fRadius += m_incRadius;

	VERTEX_3D* pVtx = nullptr;// 頂点情報へのポインタ

	int nCnt = 0;
	
	// 頂点バッファをロックし、頂点情報へのポインタを取得
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	for (int nCntV = 0; nCntV < MESHCYLINDER_V + 1; nCntV++)
	{
		for (int nCntH = 0; nCntH < MESHCYLINDER_H + 1; nCntH++)
		{
			float Angle = (D3DX_PI * 2.0f) / MESHCYLINDER_H * nCntH;

			pVtx[nCnt].pos = D3DXVECTOR3(
				sinf(Angle) * m_fRadius - sinf(Angle) * nCntV * m_fLineweight,
				0.0f,
				cosf(Angle) * m_fRadius - cosf(Angle) * nCntV * m_fLineweight
			);

			// 頂点カラーの設定
			float alpha;
			alpha = fAlpha * (1.0f - nCntV);
			pVtx[nCnt].col = D3DXCOLOR(m_col.r,m_col.g,m_col.b, alpha);

			nCnt++;
		}
	}

	// 頂点バッファをアンロックする
	m_pVtxBuff->Unlock();
}
//=============================================================================
// 描画処理
//=============================================================================
void CMeshCylinder::Draw(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// カリング設定を無効化
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);         // ライトを無効にする

	// αブレンディングを加算合成に設定
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	// αテストを有効
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);//デフォルトはfalse
	pDevice->SetRenderState(D3DRS_ALPHAREF, 0);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);//0より大きかったら描画

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
	pDevice->SetTexture(0, nullptr);

	// ポリゴンの描画
	pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, MESHCYLINDER_VERTEX, 0, MESHCYLINDER_PRIMITIVE);

	// αテストを無効に戻す
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);//デフォルトはfalse

	// カリング設定を有効化
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	// αテストを無効に戻す
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);//デフォルトはfalse

	// αブレンディングを元に戻す
	pDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
	pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);          // ライトを有効にする
}
