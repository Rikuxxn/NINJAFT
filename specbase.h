//=============================================================================
//
// スペックベース処理 [specbase.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _SPECBASE_H_// このマクロ定義がされていなかったら
#define _SPECBASE_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "motion.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CPlayer;

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
// プレイヤーがHP少ない
//*****************************************************************************
class IsHpFew : public Specification <CPlayer>
{
public:
	IsHpFew() {}
	~IsHpFew() {}

	bool IsSatisfiedBy(const CPlayer& player) const override
	{
		// HPの取得
		float nLife = player.GetHp();

		return nLife <= LIFE_THRESHOLD;
	}

private:
	static constexpr float LIFE_THRESHOLD = 5.0f;// HPの閾値
};

//*****************************************************************************
// プレイヤーがステルス状態ではない
//*****************************************************************************
class IsNotStealthSpec : public Specification<CPlayer>
{
public:
	bool IsSatisfiedBy(const CPlayer& player) const override
	{
		return !player.IsStealth();
	}
};

//*****************************************************************************
// プレイヤーがステルス状態
//*****************************************************************************
class IsStealthSpec : public Specification<CPlayer>
{
public:
	bool IsSatisfiedBy(const CPlayer& player) const override
	{
		return player.IsStealth();
	}
};

//*****************************************************************************
// プレイヤーが移動中
//*****************************************************************************
class IsMovingSpec : public Specification<CPlayer>
{
public:
	bool IsSatisfiedBy(const CPlayer& player) const override
	{
		return player.GetIsMoving();
	}
};

//*****************************************************************************
// プレイヤーがダメージモーション中ではない
//*****************************************************************************
class IsNotDamageMotionSpec : public Specification<CPlayer>
{
public:
	bool IsSatisfiedBy(const CPlayer& player) const override
	{
		return !player.GetMotion()->IsCurrentMotion(CPlayer::DAMAGE);
	}
};

#endif