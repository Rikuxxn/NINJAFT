//=============================================================================
//
// ランキング処理 [ranking.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _RANKING_H_// このマクロ定義がされていなかったら
#define _RANKING_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "scene.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CRankItem;
class CRankTime;
class CRankingManager;


//*****************************************************************************
// ランキングクラス
//*****************************************************************************
class CRanking : public CScene
{
public:
	CRanking();
	~CRanking();

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

	void TimeRanking(CRankingManager* pRankingManager);
	void ItemRanking(CRankingManager* pRankingManager);

private:
	CRankTime*			m_pRankTime;		// クリアタイムランキング表示用
	CRankItem*			m_pRankItem;		// アイテムランキング表示用
	CRankingManager*	m_pRankingManager;	// ランキングへのポインタ
};


#endif

