//=============================================================================
//
// ランキングタイム処理 [ranktime.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _RANKTIME_H_// このマクロ定義がされていなかったら
#define _RANKTIME_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"
#include "number.h"
#include "rankingmanager.h"
#include "time.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CRankColon;

//*****************************************************************************
// ランキングタイムクラス
//*****************************************************************************
class CRankTime : public CObject
{
public:
	CRankTime(int nPriority = 7);
	~CRankTime();

	static CRankTime* Create(float baseX, float baseY, float digitWidth, float digitHeight);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

	void SetRankList(const std::vector<std::pair<int, int>>& rankList);
	void ShowNewRankEffect(int rankIndex);

private:
	static constexpr int MaxRanking = 5;		// 表示数
	static constexpr int DIGITS		= 4;		// 桁数(分2、秒2)

	CNumber*	m_apNumber[MaxRanking][DIGITS];	// ナンバーへのポインタ
	int			m_nMinutes;						// 分
	int			m_nSeconds;						// 秒
	float		m_digitWidth;					// 数字1桁あたりの幅
	float		m_digitHeight;					// 数字1桁あたりの高さ
	D3DXVECTOR3 m_basePos;						// 表示の開始位置
	CRankColon* m_apRankClon[MaxRanking];		// コロンへのポインタ
	int			m_nIdxTexture;					// テクスチャインデックス

};

//*****************************************************************************
// ランキングコロンクラス
//*****************************************************************************
class CRankColon : public CObject
{
public:
	CRankColon(int nPriority = 7);
	~CRankColon();

	static CRankColon* Create(D3DXVECTOR3 pos, float fWidth, float fHeight);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

private:
	LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff;		// 頂点バッファ
	D3DXVECTOR3				m_pos;			// 位置
	float					m_fWidth;		// 幅
	float					m_fHeight;		// 高さ
	int						m_nIdxTexture;	// テクスチャインデックス
};

#endif