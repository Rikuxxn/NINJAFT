//=============================================================================
//
// リザルトタイム処理 [resulttime.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "resulttime.h"
#include "renderer.h"
#include "manager.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CResultTime::CResultTime(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	for (int nCnt = 0; nCnt < DIGITS; nCnt++)
	{
		m_apNumber[nCnt] = {};					// 各桁の数字表示用
	}
	m_nMinutes		= 0;						// 分
	m_nSeconds		= 0;						// 秒
	m_nFrameCount	= 0;						// フレームカウント
	m_digitWidth	= 0.0f;						// 数字1桁あたりの幅
	m_digitHeight	= 0.0f;						// 数字1桁あたりの高さ
	m_basePos		= INIT_VEC3;				// 表示の開始位置
	m_pColon		= NULL;						// コロン
	m_nIdxTexture	= NULL;						// テクスチャインデックス
}
//=============================================================================
// デストラクタ
//=============================================================================
CResultTime::~CResultTime()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CResultTime* CResultTime::Create(int minutes, int seconds, float baseX, float baseY, float digitWidth, float digitHeight)
{
	CResultTime* pTime = new CResultTime;

	// nullptrだったら
	if (pTime == nullptr)
	{
		return nullptr;
	}

	pTime->m_nMinutes = minutes;
	pTime->m_nSeconds = seconds;
	pTime->m_nFrameCount = 0;
	pTime->m_basePos = D3DXVECTOR3(baseX, baseY, 0.0f);
	pTime->m_digitWidth = digitWidth;
	pTime->m_digitHeight = digitHeight;

	// 初期化失敗時
	if (FAILED(pTime->Init()))
	{
		return nullptr;
	}

	return pTime;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CResultTime::Init(void)
{
	// 分の数字（0,1桁）
	for (int nCnt = 0; nCnt < 2; nCnt++)
	{
		float posX = m_basePos.x + nCnt * m_digitWidth;
		float posY = m_basePos.y;

		// ナンバーの生成
		m_apNumber[nCnt] = CNumber::Create(posX, posY, m_digitWidth, m_digitHeight);

		if (!m_apNumber[nCnt])
		{
			return E_FAIL;
		}
	}

	// コロンの生成
	CColon::Create(D3DXVECTOR3(m_basePos.x + 2 * m_digitWidth, m_basePos.y, 0.0f), m_digitWidth / 2, m_digitHeight, true);

	// コロンの幅
	float colonWidth = m_digitWidth / 2;

	// 秒の数字はコロンの幅だけ右にずらす
	for (int nCnt = 2; nCnt < DIGITS; nCnt++)
	{
		float posX = m_basePos.x + colonWidth + nCnt * m_digitWidth;
		float posY = m_basePos.y;

		m_apNumber[nCnt] = CNumber::Create(posX, posY, m_digitWidth, m_digitHeight);

		if (!m_apNumber[nCnt])
		{
			return E_FAIL;
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
void CResultTime::Uninit(void)
{
	for (int nCnt = 0; nCnt < DIGITS; nCnt++)
	{
		if (m_apNumber[nCnt] != nullptr)
		{
			// ナンバーの終了処理
			m_apNumber[nCnt]->Uninit();

			delete m_apNumber[nCnt];
			m_apNumber[nCnt] = nullptr;
		}
	}

	// オブジェクトの破棄(自分自身)
	this->Release();
}
//=============================================================================
// 更新処理
//=============================================================================
void CResultTime::Update(void)
{
	// 各桁の表示値を計算
	int min10 = m_nMinutes / 10;
	int min1 = m_nMinutes % 10;
	int sec10 = m_nSeconds / 10;
	int sec1 = m_nSeconds % 10;

	// ナンバーオブジェクトに反映
	if (m_apNumber[0]) m_apNumber[0]->SetDigit(min10);
	if (m_apNumber[1]) m_apNumber[1]->SetDigit(min1);
	if (m_apNumber[2]) m_apNumber[2]->SetDigit(sec10);
	if (m_apNumber[3]) m_apNumber[3]->SetDigit(sec1);

	for (int nCnt = 0; nCnt < 4; nCnt++)
	{
		m_apNumber[nCnt]->Update();  // UV更新
	}
}
//=============================================================================
// 描画処理
//=============================================================================
void CResultTime::Draw(void)
{
	// テクスチャの取得
	CTexture* pTexture = CManager::GetTexture();

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	for (int nCnt = 0; nCnt < DIGITS; nCnt++)
	{
		if (m_apNumber[nCnt])
		{
			// テクスチャの設定
			pDevice->SetTexture(0, pTexture->GetAddress(m_nIdxTexture));

			m_apNumber[nCnt]->Draw();
		}
	}
}
