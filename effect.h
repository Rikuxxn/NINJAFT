//=============================================================================
//
// エフェクト処理 [effect.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _EFFECT_H_// このマクロ定義がされていなかったら
#define _EFFECT_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "objectbillboard.h"
#include "block.h"

// エフェクト構造体
struct EffectDesc
{
	const char*	  path			= nullptr;
	D3DXVECTOR3   pos			= INIT_VEC3;
	D3DXVECTOR3   move			= INIT_VEC3;
	D3DXCOLOR     col			= INIT_XCOL;
	float         fRadius		= 0.0f;
	int           nLife			= 0;
	float         fGravity		= 0.0f;
	float         fDecRadius	= 0.0f;
};

//*****************************************************************************
// エフェクトクラス
//*****************************************************************************
class CEffect : public CObjectBillboard
{
public:
	CEffect(int nPriority = 5);
	~CEffect();

	static CEffect* Create(const EffectDesc& desc);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	void SetMove(D3DXVECTOR3 move) { m_move = move; }
	void SetLife(int nLife) { m_nLife = nLife; }
	void SetRadius(float fRadius) { m_fRadius = fRadius; }
	void SetGravity(float fGrav) { m_fGravity = fGrav; }
	void SetDecRadius(float fDecRad) { m_fDecRadius = fDecRad; }
	float GetRadius(void) { return m_fRadius; }
	float GetGravity(void) { return m_fGravity; }

private:
	D3DXVECTOR3 m_move;		// 移動量
	float m_fRadius;		// 半径
	int m_nLife;			// 寿命
	int m_nIdxTexture;		// テクスチャインデックス
	float m_fGravity;		// 重力
	float m_fDecRadius;		// 半径の減衰量
};

//*****************************************************************************
// モデルエフェクトクラス
//*****************************************************************************
class CModelEffect : public CObjectX
{
public:
	CModelEffect();
	~CModelEffect();

	static CModelEffect* Create(const char* path, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 velocity, D3DXVECTOR3 size, 
		int nLife, float fGravity, float scaleDec);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);

	void SetVelocity(D3DXVECTOR3 velocity) { m_velocity = velocity; }
	void SetGravity(float fGrav) { m_fGravity = fGrav; }
	void SetScaleDec(float fDec) { m_scaleDec = fDec; }
	void SetLife(int nLife) { m_nLife = nLife; }

private:
	int m_nLife;			// 寿命
	float m_fGravity;		// 重力加速度
	float m_scaleDec;		// サイズ減衰率
	D3DXVECTOR3 m_velocity; // 速度
};

#endif