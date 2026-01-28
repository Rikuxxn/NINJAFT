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
	static constexpr float	DIST_MAX	= 35.0f;				// 判定距離
	static constexpr int	MAX_ANG		= 60;					// 最大傾き角度
	static constexpr float	STIFFNESS	= 0.12f;				// バネの強さ
	static constexpr float	DAMPING		= 0.75f;				// 減衰率
	static constexpr float	WIND_AMP	= D3DXToRadian(4.0f);	// 揺れ幅
	static constexpr float	WIND_FREQ	= 1.2f;					// 揺れ速度
	static constexpr float	WIND_SPEED	= 0.016f;				// 加算量(風の強さ)
	static constexpr float	FREQ_RATE	= 0.5f;					// 揺れの間隔

	D3DXVECTOR3 m_rotVel;	// 傾き
	float		m_distMax;	// 判定距離
	bool		m_prevIn;	// 直前に入ったか
	float		m_windTime;	// 草の揺れタイマー

};

#endif
