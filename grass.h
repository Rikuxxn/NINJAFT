//=============================================================================
//
// 草処理 [grass.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _GRASS_H_
#define _GRASS_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "block.h"

//*****************************************************************************
// 草クラス
//*****************************************************************************
class CGrassBlock : public CBlock
{
public:
	CGrassBlock(int nPriority = 5);
	~CGrassBlock();

	static TYPE GetStaticType(void) { return TYPE_GRASS; }

	void Update(void);
	void Draw(void);

	int GetCollisionFlags(void) const override { return btCollisionObject::CF_NO_CONTACT_RESPONSE; }
	float GetMaxTiltDistance(void) const { return m_distMax; }
private:
	static constexpr float	DIST_MAX	= 35.0f;	// 判定距離
	static constexpr int	MAX_ANG		= 60;		// 最大傾き角度
	static constexpr float	STIFFNESS	= 0.12f;	// バネの強さ
	static constexpr float	DAMPING		= 0.75f;	// 減衰率

	D3DXVECTOR3 m_rotVel;	// 傾き
	float		m_distMax;	// 判定距離
	bool		m_prevIn;	// 直前に入ったか
};

#endif
