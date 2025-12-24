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
// ランキングアイテムクラス
//*****************************************************************************
class CRankItem : public CObject
{
public:
	CRankItem(int nPriority = 7);
	~CRankItem();

	static CRankItem* Create(float baseX, float baseY, float digitWidth, float digitHeight);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

	D3DXVECTOR3 GetPos(void) { return D3DXVECTOR3(); }

	void SetRankList(const std::vector<int>& itemList);
	void ShowNewRankEffect(int rankIndex);

private:
	static constexpr int MaxRanking = 5;			// 表示数
	static constexpr int MAX_DIGITS = 3;			// 桁数

	CNumber*	m_apNumber[MaxRanking][MAX_DIGITS];	// 各桁の数字表示用
	int			m_nDig[MaxRanking][MAX_DIGITS];		// 桁表示
	float		m_digitWidth;						// 数字1桁あたりの幅
	float		m_digitHeight;						// 数字1桁あたりの高さ
	D3DXVECTOR3 m_basePos;							// 表示の開始位置
	int			m_nIdxTexture;						// テクスチャインデックス
};

#endif

