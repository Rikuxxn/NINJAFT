//=============================================================================
//
// ランキングアイテム処理 [rankitem.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _RANKITEM_H_// このマクロ定義がされていなかったら
#define _RANKITEM_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"
#include "number.h"
#include "rankingmanager.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CRank;


//*****************************************************************************
// ランキングアイテムクラス
//*****************************************************************************
class CRankItem : public CObject
{
public:
	CRankItem(int nPriority = 7);
	~CRankItem();

	// レイアウト構造体
	struct RankLayout
	{
		float anchorX;          // 0.0～1.0
		float anchorY;          // 0.0～1.0
		float digitWidthRate;   // 画面幅に対する割合
		float digitHeightRate;  // 画面高に対する割合
		float lineSpacingRate;  // 行間（digitHeight基準）
	};

	static CRankItem* Create(float anchorX, float anchorY, float digitWidthRate, float digitHeightRate, float SpaceRate);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	int DigitNum(int nCount);

	D3DXVECTOR3 GetPos(void) { return D3DXVECTOR3(); }

	void SetRankList(const std::vector<int>& itemList);
	void ShowNewRankEffect(int rankIndex);

private:
	static constexpr int	MaxRanking		= 5;	// 表示数
	static constexpr int	MAX_DIGITS		= 3;	// 桁数
	static constexpr float	SPACING_RANK_X	= 0.2f;	// 順位と個数の間の間隔

	CNumber*	m_apNumber[MaxRanking][MAX_DIGITS];	// 各桁の数字表示用
	int			m_nDig[MaxRanking][MAX_DIGITS];		// 桁表示
	float		m_digitWidth;						// 数字1桁あたりの幅
	float		m_digitHeight;						// 数字1桁あたりの高さ
	D3DXVECTOR3 m_basePos;							// 表示の開始位置
	int			m_nIdxTexture;						// テクスチャインデックス
	RankLayout	m_layout;							// レイアウト構造体変数
	CRank*		m_apRank[MaxRanking];				// 順位へのポインタ

};

#endif

