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
#include "object3D.h"
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
	D3DXVECTOR3 m_rotVel;	// 角速度
	float m_distMax;
};

#endif
