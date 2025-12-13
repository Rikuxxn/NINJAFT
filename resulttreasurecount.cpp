//=============================================================================
//
// 宝獲得数表示処理 [resulttreasurecount.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "resulttreasurecount.h"
#include "texture.h"
#include "manager.h"


//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
int CResultTreasureCount::m_nTreasureCount = 0;
int CResultTreasureCount::m_nDig[MAX_DIGITS] = {};

//=============================================================================
// コンストラクタ
//=============================================================================
CResultTreasureCount::CResultTreasureCount(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	for (int nCnt = 0; nCnt < MAX_DIGITS; nCnt++)
	{
		m_apNumber[nCnt] = {};
	}
	m_nTreasureCount = 0;		// 音発生数
	m_digitWidth = 0.0f;		// 数字1桁あたりの幅
	m_digitHeight = 0.0f;		// 数字1桁あたりの高さ
	m_basePos = INIT_VEC3;		// 表示の開始位置
	m_nIdxTexture = 0;			// テクスチャインデックス
}
//=============================================================================
// デストラクタ
//=============================================================================
CResultTreasureCount::~CResultTreasureCount()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CResultTreasureCount* CResultTreasureCount::Create(float baseX, float baseY, float digitWidth, float digitHeight)
{
	CResultTreasureCount* resultTreasureCount = nullptr;

	resultTreasureCount = new CResultTreasureCount;

	resultTreasureCount->m_basePos = D3DXVECTOR3(baseX, baseY, 0.0f);
	resultTreasureCount->m_digitWidth = digitWidth;
	resultTreasureCount->m_digitHeight = digitHeight;

	// 初期化処理
	resultTreasureCount->Init();

	return resultTreasureCount;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CResultTreasureCount::Init(void)
{
	for (int nCnt = 0; nCnt < MAX_DIGITS; nCnt++)
	{
		float posX = m_basePos.x + nCnt * m_digitWidth;
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
void CResultTreasureCount::Uninit(void)
{
	for (int nCnt = 0; nCnt < MAX_DIGITS; nCnt++)
	{
		if (m_apNumber[nCnt] != nullptr)
		{
			// ナンバーの終了処理
			m_apNumber[nCnt]->Uninit();

			delete m_apNumber[nCnt];
			m_apNumber[nCnt] = nullptr;
		}
	}

	this->Release();
}
//=============================================================================
// 更新処理
//=============================================================================
void CResultTreasureCount::Update(void)
{
	// 各桁の数字を抽出
	for (int nCount = 0; nCount < MAX_DIGITS; nCount++)
	{
		m_nDig[nCount] = NULL;
	}

	int nSoundCount = m_nTreasureCount;
	for (int nCount = 0; nCount < MAX_DIGITS; nCount++)
	{
		int Idx = MAX_DIGITS - 1 - nCount;
		if (Idx >= 0 && Idx < MAX_DIGITS)
		{
			m_nDig[Idx] = nSoundCount % 10;
			if (nSoundCount != 0)
			{
				nSoundCount /= 10;
			}
		}
	}

	for (int nCnt = 0; nCnt < MAX_DIGITS; nCnt++)
	{
		if (m_apNumber[nCnt])
		{
			// ナンバーの更新処理
			m_apNumber[nCnt]->Update();
		}
	}
}
//=============================================================================
// 描画処理
//=============================================================================
void CResultTreasureCount::Draw(void)
{
	for (int nCnt = 0; nCnt < MAX_DIGITS; nCnt++)
	{
		int digit = m_nDig[nCnt];

		if (digit != 0 || nCnt == MAX_DIGITS - 1)
		{
			if (m_apNumber[nCnt])
			{
				// 桁設定処理
				m_apNumber[nCnt]->SetDigit(digit);

				// テクスチャの取得
				CTexture* pTexture = CManager::GetTexture();

				// デバイスの取得
				CRenderer* renderer = CManager::GetRenderer();
				LPDIRECT3DDEVICE9 pDevice = renderer->GetDevice();

				// テクスチャの設定
				pDevice->SetTexture(0, pTexture->GetAddress(m_nIdxTexture));

				// ナンバーの描画処理
				m_apNumber[nCnt]->Draw();
			}
		}
	}
}