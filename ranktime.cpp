//=============================================================================
//
// ランキングタイム処理 [ranktime.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "ranktime.h"
#include "renderer.h"
#include "manager.h"
#include "time.h"
#include "rank.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CRankTime::CRankTime(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	memset(m_apNumber, 0, sizeof(m_apNumber));		// 各桁の数字表示用
	memset(m_apRankClon, 0, sizeof(m_apRankClon));	// コロンへのポインタ
	m_nMinutes		= 0;							// 分
	m_nSeconds		= 0;							// 秒
	m_digitWidth	= 0.0f;							// 数字1桁あたりの幅
	m_digitHeight	= 0.0f;							// 数字1桁あたりの高さ
	m_basePos		= INIT_VEC3;					// 表示の開始位置
	m_nIdxTexture	= 0;							// テクスチャインデックス
}
//=============================================================================
// デストラクタ
//=============================================================================
CRankTime::~CRankTime()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CRankTime* CRankTime::Create(float baseX,float baseY,float digitWidth,float digitHeight)
{
	CRankTime* pRankTime = new CRankTime;

	// nullptrだったら
	if (pRankTime == nullptr)
	{
		return nullptr;
	}

	pRankTime->m_basePos = D3DXVECTOR3(baseX, baseY, 0.0f);
	pRankTime->m_digitWidth = digitWidth;
	pRankTime->m_digitHeight = digitHeight;

	// 初期化失敗時
	if (FAILED(pRankTime->Init()))
	{
		return nullptr;
	}

	return pRankTime;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CRankTime::Init(void)
{
	// 5位まで生成する
	for (int i = 0; i < MaxRanking; i++)
	{
		// 順位UIの生成
		float UIbaseX = m_basePos.x + m_digitWidth;
		float UIbaseY = m_basePos.y + i * (m_digitHeight + 50.0f);

		// 順位表示
		CRank::Create(D3DXVECTOR3(UIbaseX, UIbaseY, 0.0f), m_digitWidth / 2, m_digitHeight, (float)i);

		// 順位UIの幅
		float rankWidth = (m_digitWidth / 2) + 50.0f;

		// 分
		for (int n = 0; n < 2; n++)
		{
			float x = UIbaseX + rankWidth + n * m_digitWidth;
			float y = UIbaseY;

			m_apNumber[i][n] = CNumber::Create(x, y, m_digitWidth, m_digitHeight);
		}
		
		// コロンの生成（分と秒の間）
		float colonX = UIbaseX + rankWidth + 2 * m_digitWidth;
		float colonY = UIbaseY;
		m_apRankClon[i] = CRankColon::Create(D3DXVECTOR3(colonX, colonY, 0.0f), m_digitWidth / 2, m_digitHeight);

		// コロンの幅
		float colonWidth = rankWidth + m_digitWidth / 2;

		// 秒
		for (int n = 2; n < DIGITS; n++)
		{
			float x = UIbaseX + colonWidth + n * m_digitWidth;
			float y = UIbaseY;

			m_apNumber[i][n] = CNumber::Create(x, y, m_digitWidth, m_digitHeight);
		}
	}

	// テクスチャ割り当て
	CTexture* pTexture = CManager::GetTexture();
	m_nIdxTexture = pTexture->RegisterDynamic("data/TEXTURE/num_01.png");

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CRankTime::Uninit(void)
{
	for (int i = 0; i < MaxRanking; i++)
	{
		for (int n = 0; n < DIGITS; n++)
		{
			if (m_apNumber[i][n])
			{
				// ナンバーの終了処理
				m_apNumber[i][n]->Uninit();

				delete m_apNumber[i][n];
				m_apNumber[i][n] = nullptr;
			}
		}
	}

	// オブジェクトの破棄(自分自身)
	this->Release();
}
//=============================================================================
// 更新処理
//=============================================================================
void CRankTime::Update(void)
{
	for (int i = 0; i < MaxRanking; i++)
	{
		for (int n = 0; n < DIGITS; n++)
		{
			if (m_apNumber[i][n])
			{
				// ナンバーの更新処理(UV)
				m_apNumber[i][n]->Update();
			}
		}
	}
}
//=============================================================================
// 描画処理
//=============================================================================
void CRankTime::Draw(void)
{
	// テクスチャの取得
	CTexture* pTexture = CManager::GetTexture();

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	for (int i = 0; i < MaxRanking; i++)
	{
		for (int n = 0; n < DIGITS; n++)
		{
			if (m_apNumber[i][n])
			{
				// テクスチャの設定
				pDevice->SetTexture(0, pTexture->GetAddress(m_nIdxTexture));

				// ナンバーの描画処理
				m_apNumber[i][n]->Draw();
			}
		}
	}
}
//=============================================================================
// ランキングリストの設定処理
//=============================================================================
void CRankTime::SetRankList(const std::vector<std::pair<int, int>>& rankList)
{
	for (size_t i = 0; i < rankList.size() && i < MaxRanking; i++)
	{
		int min10 = rankList[i].first / 10;
		int min1 = rankList[i].first % 10;
		int sec10 = rankList[i].second / 10;
		int sec1 = rankList[i].second % 10;

		if (m_apNumber[i][0])m_apNumber[i][0]->SetDigit(min10);
		if (m_apNumber[i][1])m_apNumber[i][1]->SetDigit(min1);
		if (m_apNumber[i][2])m_apNumber[i][2]->SetDigit(sec10);
		if (m_apNumber[i][3])m_apNumber[i][3]->SetDigit(sec1);
	}
}
//=============================================================================
// ランクインエフェクトの表示
//=============================================================================
void CRankTime::ShowNewRankEffect(int rankIndex)
{
	if (rankIndex < 0 || rankIndex >= MaxRanking)
	{
		return;
	}

	for (int n = 0; n < DIGITS; n++)
	{
		if (m_apNumber[rankIndex][n])
		{
			// ランクインのスケールアニメーション
			m_apNumber[rankIndex][n]->SetScaleAnim();
		}
	}
}


//=============================================================================
// コロンのコンストラクタ
//=============================================================================
CRankColon::CRankColon(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	m_pVtxBuff = nullptr;					// 頂点バッファ
	m_pos = D3DXVECTOR3(0.0f,0.0f,0.0f);	// 位置
	m_fWidth = 0.0f;						// 幅
	m_fHeight = 0.0f;						// 高さ
	m_nIdxTexture = 0;
}
//=============================================================================
// コロンのデストラクタ
//=============================================================================
CRankColon::~CRankColon()
{
	// なし
}
//=============================================================================
// コロンの生成処理
//=============================================================================
CRankColon* CRankColon::Create(D3DXVECTOR3 pos, float fWidth, float fHeight)
{
	CRankColon* pRankColon;

	pRankColon = new CRankColon;

	pRankColon->m_pos = pos;
	pRankColon->m_fWidth = fWidth;
	pRankColon->m_fHeight = fHeight;

	// 初期化処理
	pRankColon->Init();

	return pRankColon;
}
//=============================================================================
// コロンの初期化処理
//=============================================================================
HRESULT CRankColon::Init(void)
{
	// デバイス取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	m_nIdxTexture = CManager::GetTexture()->RegisterDynamic("data/TEXTURE/colon.png");

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
	pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
	pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
	pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

	// 頂点バッファをアンロックする
	m_pVtxBuff->Unlock();

	return S_OK;
}
//=============================================================================
// コロンの終了処理
//=============================================================================
void CRankColon::Uninit(void)
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
// コロンの更新処理
//=============================================================================
void CRankColon::Update(void)
{




}
//=============================================================================
// コロンの描画処理
//=============================================================================
void CRankColon::Draw(void)
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
//=============================================================================
// コロンの位置取得処理
//=============================================================================
D3DXVECTOR3 CRankColon::GetPos(void)
{
	// 使わない
	return D3DXVECTOR3();
}