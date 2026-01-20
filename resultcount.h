//=============================================================================
//
// 回数表示処理 [resultcount.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _RESULTCOUNT_H_
#define _RESULTCOUNT_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"
#include "number.h"

//*****************************************************************************
// 回数表示クラス
//*****************************************************************************
class CCount : public CObject
{
public:
	CCount(int nPriority = 6);
	~CCount();

	// レイアウト構造体
	struct Layout
	{
		float anchorX;
		float anchorY;
		float digitWidthRate;
		float digitHeightRate;
	};

	static CCount* Create(float anchorX, float anchorY,
		float digitWidthRate, float digitHeightRate, int count);

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	int DigitNum(int nCount);

	D3DXVECTOR3 GetPos(void) { return D3DXVECTOR3(); }

private:
	static const int MAX_DIGITS = 3;		// 桁数

	CNumber*	m_apNumber[MAX_DIGITS];		// 各桁の数字表示用
	int			m_nCount;					// 数
	int			m_nDig[MAX_DIGITS];			// 桁表示
	float		m_digitWidth;				// 数字1桁あたりの幅
	float		m_digitHeight;				// 数字1桁あたりの高さ
	D3DXVECTOR3 m_basePos;					// 表示の開始位置
	int			m_nIdxTexture;				// テクスチャインデックス
	Layout		m_layout;					// レイアウト構造体変数
	D3DXVECTOR3 m_layoutPos;				// レイアウト時の位置
};


#endif