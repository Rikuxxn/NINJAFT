//=============================================================================
//
// スペックベース処理 [specbase.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#include "player.h"
#ifndef _SPECBASE_H_// このマクロ定義がされていなかったら
#define _SPECBASE_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************

//*****************************************************************************
// スペックベースクラス
//*****************************************************************************
template <class T>
class CSpecBase
{
public:
	CSpecBase() {}
	virtual ~CSpecBase() {}

	virtual bool IsSatisfieBy(const T& obj) const = 0;
};

//*****************************************************************************
// プレイヤーHP条件クラス
//*****************************************************************************
class CPlayerHPAmount : public CSpecBase<CPlayer>
{
public:
	CPlayerHPAmount() {}
	~CPlayerHPAmount() {}

	bool IsSatisfieBy(const CPlayer& player) const
	{
		// HPの取得
		float nLife = player.GetHp();
		const float threshold = 5.0f;

		return nLife > threshold;
	}
};

#endif