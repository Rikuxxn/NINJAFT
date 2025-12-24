//=============================================================================
//
// 血痕処理 [blood.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _BLOOD_H_// このマクロ定義がされていなかったら
#define _BLOOD_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object3D.h"


//*****************************************************************************
// 血痕クラス
//*****************************************************************************
class CBlood : public CObject3D
{
public:
	CBlood(int nPriority = 5);
	~CBlood();

	static CBlood* Create(D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXCOLOR col, float width, float height);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

private:
	static constexpr float DEC_ALPHA = 0.004f;	// アルファ値減少量

	int		m_nIdxTexture;						// テクスチャインデックス
};

#endif

