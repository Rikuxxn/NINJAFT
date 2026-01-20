//=============================================================================
//
// ビルボード処理 [objectBillboard.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "objectBillboard.h"
#include "renderer.h"
#include "manager.h"
#include "player.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CObjectBillboard::CObjectBillboard(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	memset(m_szPath, 0, sizeof(m_szPath));	// ファイルパス
	m_pVtxBuff		= nullptr;				// 頂点バッファへのポインタ
	m_pos			= INIT_VEC3;			// 位置
	m_rot			= INIT_VEC3;			// 向き
	m_col			= INIT_XCOL;			// 色
	m_mtxWorld		= {};					// ワールドマトリックス
	m_fSize			= 0.0f;					// サイズ(エフェクト半径)
	m_fWidth		= 0.0f;					// サイズ
	m_fHeight		= 0.0f;					// サイズ(ビルボード)
	m_nIdxTexture	= 0;					// テクスチャインデックス
}
//=============================================================================
// デストラクタ
//=============================================================================
CObjectBillboard::~CObjectBillboard()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CObjectBillboard::Init(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// テクスチャの登録
	m_nIdxTexture = CManager::GetTexture()->RegisterDynamic(m_szPath);

	// 頂点バッファの生成
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * 4,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_3D,
		D3DPOOL_MANAGED,
		&m_pVtxBuff,
		NULL);

	VERTEX_3D* pVtx = nullptr;// 頂点情報へのポインタ

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	// 頂点座標の設定
	pVtx[0].pos = D3DXVECTOR3(-m_fSize, +m_fSize + m_fHeight, 0.0f);
	pVtx[1].pos = D3DXVECTOR3(+m_fSize, +m_fSize + m_fHeight, 0.0f);
	pVtx[2].pos = D3DXVECTOR3(-m_fSize, -m_fSize - m_fHeight, 0.0f);
	pVtx[3].pos = D3DXVECTOR3(+m_fSize, -m_fSize - m_fHeight, 0.0f);

	// 各頂点の法線の設定
	pVtx[0].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[1].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[2].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);
	pVtx[3].nor = D3DXVECTOR3(0.0f, 0.0f, -1.0f);

	// 頂点カラーの設定
	pVtx[0].col = m_col;
	pVtx[1].col = m_col;
	pVtx[2].col = m_col;
	pVtx[3].col = m_col;

	// テクスチャ座標の設定
	pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
	pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
	pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

	// 頂点バッファをアンロックする
	m_pVtxBuff->Unlock();

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CObjectBillboard::Uninit(void)
{
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
void CObjectBillboard::Update(void)
{
	VERTEX_3D* pVtx;// 頂点情報へのポインタ

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);
	
	// 頂点座標の設定
	pVtx[0].pos = D3DXVECTOR3(-m_fSize, +m_fSize + m_fHeight, 0.0f);
	pVtx[1].pos = D3DXVECTOR3(+m_fSize, +m_fSize + m_fHeight, 0.0f);
	pVtx[2].pos = D3DXVECTOR3(-m_fSize, -m_fSize - m_fHeight, 0.0f);
	pVtx[3].pos = D3DXVECTOR3(+m_fSize, -m_fSize - m_fHeight, 0.0f);

	pVtx[0].col = m_col;
	pVtx[1].col = m_col;
	pVtx[2].col = m_col;
	pVtx[3].col = m_col;

	// 頂点バッファをアンロックする
	m_pVtxBuff->Unlock();
}
//=============================================================================
// 更新処理(回転)
//=============================================================================
void CObjectBillboard::UpdateTurn(void)
{
	VERTEX_3D* pVtx;// 頂点情報へのポインタ

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	float c = cosf(m_rot.z);
	float s = sinf(m_rot.z);

	// 元の頂点を回転
	for (int nCnt = 0; nCnt < 4; nCnt++)
	{
		float x = pVtx[nCnt].pos.x;
		float y = pVtx[nCnt].pos.y;

		pVtx[nCnt].pos.x = x * c - y * s;
		pVtx[nCnt].pos.y = x * s + y * c;

		// 色の更新
		pVtx[nCnt].col = m_col;
	}

	// 頂点バッファをアンロックする
	m_pVtxBuff->Unlock();
}
//=============================================================================
// 描画処理
//=============================================================================
void CObjectBillboard::Draw(void)
{
	// テクスチャの取得
	CTexture* pTexture = CManager::GetTexture();

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// ライトを無効にする
	pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	// Zテスト
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);	// Zの比較方法
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);		// Zバッファに書き込まない

	// 計算用マトリックス
	D3DXMATRIX mtxRot, mtxTrans;

	// ワールドマトリックスの初期化
	D3DXMatrixIdentity(&m_mtxWorld);

	// ビューマトリックスの取得
	D3DXMATRIX mtxView;
	pDevice->GetTransform(D3DTS_VIEW, &mtxView);

	// カメラの逆行列を設定
	m_mtxWorld._11 = mtxView._11;
	m_mtxWorld._12 = mtxView._21;
	m_mtxWorld._13 = mtxView._31;
	m_mtxWorld._21 = mtxView._12;
	m_mtxWorld._22 = mtxView._22;
	m_mtxWorld._23 = mtxView._32;
	m_mtxWorld._31 = mtxView._13;
	m_mtxWorld._32 = mtxView._23;
	m_mtxWorld._33 = mtxView._33;

	// 位置を反映
	D3DXMatrixTranslation(&mtxTrans, m_pos.x, m_pos.y, m_pos.z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxTrans);

	// ワールドマトリックスを設定
	pDevice->SetTransform(D3DTS_WORLD, &m_mtxWorld);

	// 頂点バッファをデータストリームに設定
	pDevice->SetStreamSource(0, m_pVtxBuff, 0, sizeof(VERTEX_3D));

	// 頂点フォーマットの設定
	pDevice->SetFVF(FVF_VERTEX_3D);

	// テクスチャの設定
	pDevice->SetTexture(0, pTexture->GetAddress(m_nIdxTexture));

	// ポリゴンの描画
	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

	// 元に戻す
	pDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);//Zの比較方法
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);//Zバッファに書き込む

	// ライトを有効にする
	pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
}
//=============================================================================
// 3Dゲージの頂点座標更新処理
//=============================================================================
void CObjectBillboard::UpdateGuageVtx(float fRate)
{
	VERTEX_3D* pVtx;// 頂点情報へのポインタ

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	float totalWidth = m_fWidth;
	float currentWidth = totalWidth * fRate;
	float halfHeight = m_fHeight;

	// ビルボードローカル座標で右端を中心に固定
	pVtx[0].pos = D3DXVECTOR3(-m_fWidth * 0.5f, halfHeight, 0.0f); // 左上
	pVtx[1].pos = D3DXVECTOR3(currentWidth - m_fWidth * 0.5f, halfHeight, 0.0f); // 右上
	pVtx[2].pos = D3DXVECTOR3(-m_fWidth * 0.5f, -halfHeight, 0.0f); // 左下
	pVtx[3].pos = D3DXVECTOR3(currentWidth - m_fWidth * 0.5f, -halfHeight, 0.0f); // 右下

	pVtx[0].col = m_col;
	pVtx[1].col = m_col;
	pVtx[2].col = m_col;
	pVtx[3].col = m_col;

	// 頂点バッファをアンロックする
	m_pVtxBuff->Unlock();
}
//=============================================================================
// フレームの頂点座標更新処理
//=============================================================================
void CObjectBillboard::UpdateFrame(void)
{
	VERTEX_3D* pVtx;// 頂点情報へのポインタ

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	float halfWidth = m_fWidth * 0.5f;
	float halfHeight = m_fHeight;

	float offset = 1.0f; // フレームの線幅分のオフセット

	// ゲージと同じ中央揃え
	pVtx[0].pos = D3DXVECTOR3(-halfWidth - offset, halfHeight + offset, 0.0f);	// 左上
	pVtx[1].pos = D3DXVECTOR3(halfWidth + offset, halfHeight + offset, 0.0f);	// 右上
	pVtx[2].pos = D3DXVECTOR3(-halfWidth - offset, -halfHeight - offset, 0.0f);	// 左下
	pVtx[3].pos = D3DXVECTOR3(halfWidth + offset, -halfHeight - offset, 0.0f);	// 右下

	pVtx[0].col = m_col;
	pVtx[1].col = m_col;
	pVtx[2].col = m_col;
	pVtx[3].col = m_col;

	// 頂点バッファをアンロックする
	m_pVtxBuff->Unlock();
}
