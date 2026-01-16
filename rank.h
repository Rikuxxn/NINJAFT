//=============================================================================
//
// ランキング順位表示処理 [rank.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _RANK_H_// このマクロ定義がされていなかったら
#define _RANK_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"

//*****************************************************************************
// ランキング順位クラス
//*****************************************************************************
class CRank : public CObject
{
public:
	// 順位
	enum RANK
	{
		RANK_FIRST = 0,	// 1位
		RANK_SECOND,	// 2位
		RANK_THIRD,		// 3位
		RANK_OTHER,		// その他
		RANK_MAX
	};

	CRank(int nPriority = 7);
	~CRank();

	static CRank* Create(D3DXVECTOR3 pos, float fWidth, float fHeight, float fRank);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

	void SetPos(D3DXVECTOR3 pos) { m_pos = pos; }
	void SetSize(float w, float h) { m_fWidth = w; m_fHeight = h; }

private:
	LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff;		// 頂点バッファ
	D3DXVECTOR3				m_pos;			// 位置
	float					m_fWidth;		// 幅
	float					m_fHeight;		// 高さ
	float					m_fRank;		// 順位
	int						m_nIdxTexture;	// テクスチャインデックス
};

#endif