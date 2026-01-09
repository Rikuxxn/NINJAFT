//=============================================================================
//
// ナンバー処理 [number.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "number.h"
#include "renderer.h"
#include "manager.h"
#include "easing.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CNumber::CNumber()
{
	// 値のクリア
	m_pVtxBuff	= nullptr;	// 頂点バッファへのポインタ
	m_pos		= INIT_VEC3;// 位置
	m_col		= INIT_COL;	// 色
	m_digit		= 0;		// 桁
	m_fWidth	= 0.0f;		// 幅
	m_fHeight	= 0.0f;		// 高さ
	m_scale		= 0.0f;		// 拡大率
	m_easeTime	= 0.0f;		// タイマー
	m_easeSpeed = 0.0f;		// tの進むスピード
	m_isExpanding = false;	// true : 拡大, false : 縮小
}
//=============================================================================
// デストラクタ
//=============================================================================
CNumber::~CNumber()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CNumber* CNumber::Create(float fposX, float fposY, float fWidth, float fHeight)
{
	// オブジェクトの生成
	CNumber* pNumber = new CNumber;

	// nullptrだったら
	if (pNumber == nullptr)
	{
		return nullptr;
	}

	// 初期化失敗時
	if (FAILED(pNumber->Init(fposX, fposY, fWidth, fHeight)))
	{
		return nullptr;
	}

	return pNumber;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CNumber::Init(float fposX, float fposY, float fWidth, float fHeight)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// 値の保存
	m_pos = D3DXVECTOR3(fposX, fposY, 0.0f);
	m_fWidth = fWidth;
	m_fHeight = fHeight;
	m_col = D3DCOLOR_ARGB(255, 255, 255, 255);
	m_digit = 0;

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
	pVtx[0].pos = D3DXVECTOR3(fposX, fposY, 0.0f);
	pVtx[1].pos = D3DXVECTOR3(fposX + fWidth, fposY, 0.0f);
	pVtx[2].pos = D3DXVECTOR3(fposX, fposY + fHeight, 0.0f);
	pVtx[3].pos = D3DXVECTOR3(fposX + fWidth, fposY + fHeight, 0.0f);

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
void CNumber::Uninit(void)
{
	// 頂点バッファの破棄
	if (m_pVtxBuff != nullptr)
	{
		m_pVtxBuff->Release();
		m_pVtxBuff = nullptr;
	}
}
//=============================================================================
// 更新処理
//=============================================================================
void CNumber::Update(void)
{
	// スケール更新（拡大縮小ループ）
	if (m_easeSpeed != 0.0f)
	{
		// 時間を進める
		m_easeTime += (m_isExpanding ? m_easeSpeed : -m_easeSpeed);

		// 上限到達 → 縮小に切り替え
		if (m_easeTime >= 1.0f)
		{
			m_easeTime = 1.0f;
			m_isExpanding = false;
		}

		// 下限到達 → 拡大に切り替え
		if (m_easeTime <= 0.0f)
		{
			m_easeTime = 0.0f;
			m_isExpanding = true;
		}

		// イージング適用
		m_scale = CEasing::Ease(1.0f, MAX_SCALE, m_easeTime, CEasing::EaseInOutSine);
	}
	else
	{
		m_scale = 1.0f;
	}

	// UV計算（1桁10分割の場合）
	const float digitWidthUV = 1.0f / 10.0f;
	float tu = m_digit * digitWidthUV;
	float tu2 = tu + digitWidthUV;

	VERTEX_2D* pVtx;

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	// 頂点座標に scale を反映（中心拡大）
	float w = m_fWidth * m_scale;
	float h = m_fHeight * m_scale;
	float cx = m_pos.x + m_fWidth * 0.5f;
	float cy = m_pos.y + m_fHeight * 0.5f;

	pVtx[0].pos = D3DXVECTOR3(cx - w * 0.5f, cy - h * 0.5f, 0.0f);
	pVtx[1].pos = D3DXVECTOR3(cx + w * 0.5f, cy - h * 0.5f, 0.0f);
	pVtx[2].pos = D3DXVECTOR3(cx - w * 0.5f, cy + h * 0.5f, 0.0f);
	pVtx[3].pos = D3DXVECTOR3(cx + w * 0.5f, cy + h * 0.5f, 0.0f);

	// UV座標の更新（テクスチャの一部を数字ごとに切り替える）
	pVtx[0].tex = D3DXVECTOR2(tu, 0.0f);   // 左上
	pVtx[1].tex = D3DXVECTOR2(tu2, 0.0f);  // 右上
	pVtx[2].tex = D3DXVECTOR2(tu, 1.0f);   // 左下
	pVtx[3].tex = D3DXVECTOR2(tu2, 1.0f);  // 右下

	// 頂点バッファをアンロックする
	m_pVtxBuff->Unlock();
}
//=============================================================================
// 描画処理
//=============================================================================
void CNumber::Draw(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// 頂点バッファをデータストリームに設定
	pDevice->SetStreamSource(0, m_pVtxBuff, 0, sizeof(VERTEX_2D));

	// 頂点フォーマットの設定
	pDevice->SetFVF(FVF_VERTEX_2D);

	// ポリゴンの描画
	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

	// テクスチャの設定
	pDevice->SetTexture(0, nullptr);
}
//=============================================================================
// 桁数設定処理
//=============================================================================
void CNumber::SetDigit(int digit)
{
	// 範囲チェック
	if (digit < 0 || digit > 9)
	{
		digit = 0;
	}

	m_digit = digit;
}
//=============================================================================
// スケールアニメーション設定処理
//=============================================================================
void CNumber::SetScaleAnim(void)
{
	m_easeTime = 0.0f;       // アニメーション開始位置
	m_isExpanding = true;    // 拡大
	m_easeSpeed = 0.03f;     // t の増加量（速度）
}
