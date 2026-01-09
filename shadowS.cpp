//=============================================================================
//
// ステンシルシャドウ処理 [shadowS.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "shadowS.h"
#include "renderer.h"
#include "manager.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CShadowS::CShadowS(int nPriority) : CObjectX(nPriority)
{
	// 値のクリア
	m_pVtxBuff	= nullptr;		// 頂点バッファへのポインタ
	m_pos		= INIT_VEC3;	// 位置
	m_col		= INIT_XCOL;	// 色
	m_fWidth	= 0.0f;			// 幅
	m_fHeight	= 0.0f;			// 高さ
}
//=============================================================================
// デストラクタ
//=============================================================================
CShadowS::~CShadowS()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CShadowS* CShadowS::Create(const char* pFilepath,D3DXVECTOR3 size)
{
	CShadowS* pShadowS = new CShadowS;

	// nullptrだったら
	if (pShadowS == nullptr)
	{
		return nullptr;
	}

	pShadowS->SetPath(pFilepath);
	pShadowS->SetSize(size);

	// 初期化失敗時
	if (FAILED(pShadowS->Init()))
	{
		return nullptr;
	}

	return pShadowS;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CShadowS::Init(void)
{
	// Xファイルオブジェクトの初期化処理
	CObjectX::Init();

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// 頂点バッファの生成
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&m_pVtxBuff,
		NULL);

	// 初期化
	m_pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_fWidth = 1920.0f;
	m_fHeight = 1080.0f;

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
	pVtx[0].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.5f);
	pVtx[1].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.5f);
	pVtx[2].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.5f);
	pVtx[3].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.5f);

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
void CShadowS::Uninit(void)
{
	// 頂点バッファの破棄
	if (m_pVtxBuff != nullptr)
	{
		m_pVtxBuff->Release();
		m_pVtxBuff = nullptr;
	}

	// Xファイルオブジェクトの破棄
	CObjectX::Uninit();
}
//=============================================================================
// 更新処理
//=============================================================================
void CShadowS::Update(void)
{
	// Xファイルオブジェクトの更新処理
	CObjectX::Update();

	// 位置の設定
	SetPos(m_pos);
}
//=============================================================================
// 描画処理
//=============================================================================
void CShadowS::Draw(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// ステンシルテストを有効にする
	pDevice->SetRenderState(D3DRS_STENCILENABLE, TRUE);

	// Zバッファへの書き込みを無効にする
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

	// カラーバッファへの書き込みを無効にする
	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0x00000000);

	// ステンシルバッファの比較パラメータを設定
	pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_ALWAYS);
	pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);	// ステンシルテスト : 合格 / Zテスト : 合格
	//pDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_INCRSAT);// ステンシルテスト : 合格 / Zテスト : 不合格
	//pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_INCRSAT);	// ステンシルテスト : 不合格

	// 表面をカリング
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CW);

	// Xオブジェクトの描画処理
	CObjectX::Draw();

	// ステンシルバッファの参照値を設定(1にする)
	pDevice->SetRenderState(D3DRS_STENCILREF, m_stencilRef);

	// ステンシルバッファの比較パラメータを設定
	pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
	pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_INCRSAT);	// ステンシルテスト : 合格 / Zテスト : 合格
	pDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP);		// ステンシルテスト : 合格 / Zテスト : 不合格
	pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_INCRSAT);	// ステンシルテスト : 不合格

	// 裏面をカリング
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

	// Xオブジェクトの描画処理
	CObjectX::Draw();

	// カラーバッファへの書き込みを有効にする
	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0000000F);

	// ステンシルバッファの参照値を設定(2にする)
	pDevice->SetRenderState(D3DRS_STENCILREF, m_stencilRef + 1);

	// ステンシルバッファの比較パラメータを設定
	pDevice->SetRenderState(D3DRS_STENCILFUNC, D3DCMP_EQUAL);
	pDevice->SetRenderState(D3DRS_STENCILPASS, D3DSTENCILOP_KEEP);	// ステンシルテスト : 合格 / Zテスト : 合格
	pDevice->SetRenderState(D3DRS_STENCILZFAIL, D3DSTENCILOP_INCRSAT);	// ステンシルテスト : 合格 / Zテスト : 不合格
	pDevice->SetRenderState(D3DRS_STENCILFAIL, D3DSTENCILOP_INCRSAT);	// ステンシルテスト : 不合格

	// 頂点バッファをデータストリームに設定
	pDevice->SetStreamSource(0, m_pVtxBuff, 0, sizeof(VERTEX_2D));

	// 頂点フォーマットの設定
	pDevice->SetFVF(FVF_VERTEX_2D);

	// 黒いポリゴンの描画
	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

	//*****************************************************************************
	// 状態を元に戻す
	//*****************************************************************************

	// ステンシルテストを無効にする
	pDevice->SetRenderState(D3DRS_STENCILENABLE, FALSE);

	// Zバッファへの書き込みを有効にする
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

	// カラーバッファへの書き込みを有効にする
	pDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0000000F);

	// カリング設定を有効化
	pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
}
