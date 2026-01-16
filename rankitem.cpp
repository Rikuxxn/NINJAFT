//=============================================================================
//
// ランキングアイテム処理 [rankitem.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "rankitem.h"
#include "texture.h"
#include "manager.h"
#include "rank.h"


//=============================================================================
// コンストラクタ
//=============================================================================
CRankItem::CRankItem(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	memset(m_apNumber, 0, sizeof(m_apNumber));	// 各桁の数字表示用
	memset(m_apRank, 0, sizeof(m_apRank));	// 各桁の順位表示用
	m_digitWidth = 0.0f;		// 数字1桁あたりの幅
	m_digitHeight = 0.0f;		// 数字1桁あたりの高さ
	m_basePos = INIT_VEC3;		// 表示の開始位置
	m_nIdxTexture = 0;			// テクスチャインデックス
}
//=============================================================================
// デストラクタ
//=============================================================================
CRankItem::~CRankItem()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CRankItem* CRankItem::Create(float anchorX, float anchorY, float digitWidthRate, float digitHeightRate, float SpaceRate)
{
	CRankItem* rankItem = new CRankItem;

	// nullptrだったら
	if (rankItem == nullptr)
	{
		return nullptr;
	}

	rankItem->m_layout.anchorX = anchorX;
	rankItem->m_layout.anchorY = anchorY;
	rankItem->m_layout.digitWidthRate = digitWidthRate;
	rankItem->m_layout.digitHeightRate = digitHeightRate;
	rankItem->m_layout.lineSpacingRate = SpaceRate;

	// 初期化失敗時
	if (FAILED(rankItem->Init()))
	{
		return nullptr;
	}

	return rankItem;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CRankItem::Init(void)
{
	// 5位まで生成する
	for (int i = 0; i < MaxRanking; i++)
	{
		// 順位UIの生成
		float UIbaseX = m_basePos.x + m_digitWidth;
		float UIbaseY = m_basePos.y + i * (m_digitHeight + 50.0f);

		// 順位表示
		m_apRank[i] = CRank::Create(D3DXVECTOR3(UIbaseX, UIbaseY, 0.0f), m_digitWidth / 2, m_digitHeight, (float)i);

		// 順位UIの幅
		float rankWidth = (m_digitWidth / 2) + 30.0f;

		// アイテム数の桁生成
		for (int n = 0; n < MAX_DIGITS; n++)
		{
			float x = UIbaseX + rankWidth + n * m_digitWidth;
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
void CRankItem::Uninit(void)
{
	for (int i = 0; i < MaxRanking; i++)
	{
		for (int n = 0; n < MAX_DIGITS; n++)
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

	this->Release();
}
//=============================================================================
// 更新処理
//=============================================================================
void CRankItem::Update(void)
{
	// バックバッファサイズの取得
	float sw = (float)CManager::GetRenderer()->GetBackBufferWidth();
	float sh = (float)CManager::GetRenderer()->GetBackBufferHeight();

	float digitW = sw * m_layout.digitWidthRate;
	float digitH = sh * m_layout.digitHeightRate;
	float lineH = digitH * m_layout.lineSpacingRate;

	// ランキング全体の開始位置
	float startX = sw * m_layout.anchorX;
	float startY = sh * m_layout.anchorY;

	for (int i = 0; i < MaxRanking; i++)
	{
		// 表示する桁数（0でも1桁表示）
		int value = m_nDig[i][0] * 100 + m_nDig[i][1] * 10 + m_nDig[i][2];
		int digitCount = (value == 0) ? 1 : DigitNum(value);

		float totalWidth = digitW * digitCount;
		float x = startX - totalWidth * 0.5f;
		float y = startY + i * lineH;

		// 順位UI
		if (m_apRank[i])
		{
			m_apRank[i]->SetPos({ x - digitW, y, 0.0f });
			m_apRank[i]->SetSize(digitW * 0.5f, digitH);
			m_apRank[i]->Update();
		}

		for (int n = 0; n < MAX_DIGITS; n++)
		{
			// 位置を代入
			D3DXVECTOR3 pos(x + digitW * SPACING_RANK_X, y, 0.0f);

			m_apNumber[i][n]->SetPos(pos);
			m_apNumber[i][n]->SetSize(digitW, digitH);

			// ナンバー更新
			m_apNumber[i][n]->Update();
		}
	}
}
//=============================================================================
// 描画処理
//=============================================================================
void CRankItem::Draw(void)
{
	// テクスチャの取得
	CTexture* pTexture = CManager::GetTexture();

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	for (int i = 0; i < MaxRanking; i++)
	{
		bool drawStarted = false;

		for (int n = 0; n < MAX_DIGITS; n++)
		{
			int digit = m_nDig[i][n]; // 各桁の値

			// 先頭0をスキップ
			if (!drawStarted)
			{
				if (digit == 0 && n != MAX_DIGITS - 1)
				{
					continue;
				}

				drawStarted = true;
			}

			if (!m_apNumber[i][n])
			{
				continue;
			}

			// 桁の設定
			m_apNumber[i][n]->SetDigit(digit);

			// テクスチャの設定
			pDevice->SetTexture(0, pTexture->GetAddress(m_nIdxTexture));

			// ナンバーの描画
			m_apNumber[i][n]->Draw();
		}
	}
}
//=============================================================================
// 桁分割処理
//=============================================================================
int CRankItem::DigitNum(int nCount)
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
//=============================================================================
// ランキングリストの設定処理
//=============================================================================
void CRankItem::SetRankList(const std::vector<int>& itemList)
{
	for (size_t i = 0; i < itemList.size() && i < MaxRanking; i++)
	{
		int value = (i < itemList.size()) ? itemList[i] : 0;

		// 最大 999 に制限
		value = std::min(value, 999);

		// 桁分解
		m_nDig[i][0] = (value / 100) % 10;
		m_nDig[i][1] = (value / 10) % 10;
		m_nDig[i][2] = value % 10;
	}
}
//=============================================================================
// ランクインエフェクトの表示
//=============================================================================
void CRankItem::ShowNewRankEffect(int rankIndex)
{
	if (rankIndex < 0 || rankIndex >= MaxRanking)
	{
		return;
	}

	for (int n = 0; n < MAX_DIGITS; n++)
	{
		if (m_apNumber[rankIndex][n])
		{
			// ランクインのスケールアニメーション
			m_apNumber[rankIndex][n]->SetScaleAnim();
		}
	}
}