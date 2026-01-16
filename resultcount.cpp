//=============================================================================
//
// 回数表示処理 [resultcount.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "resultcount.h"
#include "texture.h"
#include "manager.h"


//=============================================================================
// コンストラクタ
//=============================================================================
CCount::CCount(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	memset(m_apNumber, 0, sizeof(m_apNumber));	// 各桁の数字表示用
	m_nCount		= 0;			// 数
	m_digitWidth	= 0.0f;			// 数字1桁あたりの幅
	m_digitHeight	= 0.0f;			// 数字1桁あたりの高さ
	m_basePos		= INIT_VEC3;	// 表示の開始位置
	m_nIdxTexture	= 0;			// テクスチャインデックス
	m_layoutPos		= INIT_VEC3;	// レイアウト時の位置
}
//=============================================================================
// デストラクタ
//=============================================================================
CCount::~CCount()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CCount* CCount::Create(float anchorX, float anchorY,
	float digitWidthRate, float digitHeightRate, int count)
{
	CCount* pCount = new CCount;

	// nullptrだったら
	if (pCount == nullptr)
	{
		return nullptr;
	}

	pCount->m_layout.anchorX = anchorX;
	pCount->m_layout.anchorY = anchorY;
	pCount->m_layout.digitWidthRate = digitWidthRate;
	pCount->m_layout.digitHeightRate = digitHeightRate;
	pCount->m_nCount = count;

	// 初期化失敗時
	if (FAILED(pCount->Init()))
	{
		return nullptr;
	}

	return pCount;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CCount::Init(void)
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
void CCount::Uninit(void)
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
void CCount::Update(void)
{
	// バックバッファサイズの取得
	float sw = (float)CManager::GetRenderer()->GetBackBufferWidth();
	float sh = (float)CManager::GetRenderer()->GetBackBufferHeight();

	float digitW = sw * m_layout.digitWidthRate;
	float digitH = sh * m_layout.digitHeightRate;

	int digitCount = DigitNum(m_nCount);
	float totalWidth = digitW * digitCount;

	float startX = sw * m_layout.anchorX - totalWidth * 0.5f;
	float startY = sh * m_layout.anchorY - digitH * 0.5f;

	// 桁計算
	int nSoundCount = m_nCount;
	for (int i = 0; i < MAX_DIGITS; i++)
	{
		m_nDig[i] = 0;
	}

	for (int i = 0; i < digitCount; i++)
	{
		int idx = digitCount - 1 - i;
		m_nDig[idx] = nSoundCount % 10;
		nSoundCount /= 10;
	}

	// 数字配置
	for (int i = 0; i < digitCount; i++)
	{
		// 位置を代入する
		D3DXVECTOR3 pos(startX + i * digitW, startY, 0.0f);

		// 位置の更新
		m_apNumber[i]->SetPos(pos);

		m_apNumber[i]->SetSize(digitW, digitH);
		m_apNumber[i]->Update();
	}
}
//=============================================================================
// 描画処理
//=============================================================================
void CCount::Draw(void)
{
	for (int i = 0; i < DigitNum(m_nCount); i++)
	{
		int digit = m_nDig[i];

		// 桁の設定
		m_apNumber[i]->SetDigit(digit);

		// テクスチャの取得
		CTexture* pTexture = CManager::GetTexture();

		// デバイスの取得
		CRenderer* renderer = CManager::GetRenderer();
		LPDIRECT3DDEVICE9 pDevice = renderer->GetDevice();

		// テクスチャの設定
		pDevice->SetTexture(0, pTexture->GetAddress(m_nIdxTexture));

		// 描画
		m_apNumber[i]->Draw();
	}
}
//=============================================================================
// 桁分割処理
//=============================================================================
int CCount::DigitNum(int nCount)
{
	if (nCount == 0)
	{
		return 1;
	}

	int nCnt = 0;
	while (nCount > 0)
	{
		nCount /= 10;
		nCnt++;
	}

	return nCnt;
}