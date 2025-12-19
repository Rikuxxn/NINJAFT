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
class Specification
{
public:
	Specification() {}
	virtual ~Specification() {}

	virtual bool IsSatisfiedBy(const T& obj) const = 0;
};

//*****************************************************************************
// スペックの合成(AND)クラス
//*****************************************************************************
template <typename T>
class AndSpecification : public Specification<T> 
{
public:
	AndSpecification(const Specification<T>& a, const Specification<T>& b)
		: a(a), b(b) {}

	bool IsSatisfiedBy(const T& obj) const override
	{
		return a.IsSatisfiedBy(obj) && b.IsSatisfiedBy(obj);
	}

private:
	const Specification<T>& a;
	const Specification<T>& b;
};

//*****************************************************************************
// スペックの合成(OR)クラス
//*****************************************************************************
template <typename T>
class OrSpecification : public Specification<T> 
{
public:
	OrSpecification(const Specification<T>& a, const Specification<T>& b)
		: a(a), b(b) {}

	bool IsSatisfiedBy(const T& obj) const override 
	{
		return a.IsSatisfiedBy(obj) || b.IsSatisfiedBy(obj);
	}

private:
	const Specification<T>& a;
	const Specification<T>& b;
};

//*****************************************************************************
// スペックの合成(NOT)クラス
//*****************************************************************************
template <typename T>
class NotSpecification : public Specification<T> 
{
public:
	explicit NotSpecification(const Specification<T>& spec) : spec(spec) {}

	bool IsSatisfiedBy(const T& obj) const override
	{
		return !spec.IsSatisfiedBy(obj);
	}

private:
	const Specification<T>& spec;
};


//*****************************************************************************
// プレイヤーHP条件クラス
//*****************************************************************************
class CPlayerHPAmount : public Specification <CPlayer>
{
public:
	CPlayerHPAmount() {}
	~CPlayerHPAmount() {}

	bool IsSatisfiedBy(const CPlayer& player) const override
	{
		// HPの取得
		float nLife = player.GetHp();
		const float threshold = 5.0f;

		return nLife > threshold;
	}
};

//*****************************************************************************
// プレイヤーステルス条件クラス
//*****************************************************************************
class IsStealthSpec : public Specification <CPlayer>
{
public:
	bool IsSatisfiedBy(const CPlayer& player) const override 
	{
		return player.IsStealth();
	}
};

#endif