//=============================================================================
//
// パーティクル処理 [particle.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _PARTICLE_H_// このマクロ定義がされていなかったら
#define _PARTICLE_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "effect.h"
#include "unordered_map"
#include "functional"

//*****************************************************************************
// パーティクルクラス
//*****************************************************************************
class CParticle : public CEffect
{
public:
	CParticle();
	~CParticle();

	//*************************************************************************
	// パーティクル生成テンプレート
	//*************************************************************************
	// particleType : 生成したいパーティクルクラス名
	template <typename particleType>
	static particleType* Create(D3DXVECTOR3 dir, D3DXVECTOR3 pos, D3DXCOLOR col, int nLife, int nMaxParticle)
	{
		// 渡された型がCParticleを継承していなかった場合
		static_assert(std::is_base_of<CParticle, particleType>::value, "型はCParticleを継承していません。");

		particleType* pParticle = new particleType();// 型のインスタンス生成

		// nullptrだったら
		if (!pParticle)
		{
			return nullptr;
		}

		// 初期化失敗時
		if (FAILED(pParticle->Init()))
		{
			return nullptr;
		}

		if (dir != INIT_VEC3)
		{
			pParticle->SetDir(dir);
		}

		pParticle->SetPos(pos);
		pParticle->SetCol(col);
		pParticle->SetLife(nLife);
		pParticle->m_nMaxParticle = nMaxParticle;

		return pParticle;
	}

	HRESULT Init(void);
	void Update(void);

	void SetLife(int nLife) { m_nLife = nLife; }
	void SetDir(D3DXVECTOR3 dir) { m_Dir = dir; }

	int GetLife(void) { return m_nLife; }
	int GetMaxParticle(void) { return m_nMaxParticle; }
	D3DXVECTOR3 GetDir(void) { return m_Dir; }

private:
	D3DXVECTOR3 m_Dir;			// 方向
	int			m_nLife;		// 寿命
	int			m_nMaxParticle;	// 粒子の数
};

//*****************************************************************************
// 炎パーティクルクラス
//*****************************************************************************
class CFireParticle : public CParticle
{
public:
	CFireParticle() {}
	~CFireParticle() {}

	HRESULT Init(void);
	void Update(void);

private:

};

//*****************************************************************************
// 水しぶきパーティクルクラス
//*****************************************************************************
class CWaterParticle : public CParticle
{
public:
	CWaterParticle() {}
	~CWaterParticle() {}

	HRESULT Init(void);
	void Update(void);

private:

};

//*****************************************************************************
// 埃パーティクルクラス
//*****************************************************************************
class CDustParticle : public CParticle
{
public:
	CDustParticle() {}
	~CDustParticle() {}

	HRESULT Init(void);
	void Update(void);

private:

};

//*****************************************************************************
// 浮遊パーティクルクラス
//*****************************************************************************
class CFloatingParticle : public CParticle
{
public:
	CFloatingParticle() {}
	~CFloatingParticle() {}

	HRESULT Init(void);
	void Update(void);

private:

};

//*****************************************************************************
// 桜パーティクルクラス
//*****************************************************************************
class CBlossomParticle : public CParticle
{
public:
	CBlossomParticle() {}
	~CBlossomParticle() {}

	HRESULT Init(void);
	void Update(void);

private:

};

//*****************************************************************************
// お宝パーティクルクラス
//*****************************************************************************
class CTreasureParticle : public CParticle
{
public:
	CTreasureParticle() {}
	~CTreasureParticle() {}

	HRESULT Init(void);
	void Update(void);

private:

};

//*****************************************************************************
// 鬼火パーティクルクラス
//*****************************************************************************
class COnibiParticle : public CParticle
{
public:
	COnibiParticle() {}
	~COnibiParticle() {}

	HRESULT Init(void);
	void Update(void);

private:

};

//*****************************************************************************
// 煙パーティクルクラス
//*****************************************************************************
class CSmokeParticle : public CParticle
{
public:
	CSmokeParticle() {}
	~CSmokeParticle() {}

	HRESULT Init(void);
	void Update(void);

private:

};

//*****************************************************************************
// 血しぶきパーティクルクラス
//*****************************************************************************
class CBloodSplatter : public CParticle
{
public:
	CBloodSplatter() {}
	~CBloodSplatter() {}

	HRESULT Init(void);
	void Update(void);

private:

};

//*****************************************************************************
// ダッシュパーティクルクラス
//*****************************************************************************
class CDushParticle : public CParticle
{
public:
	CDushParticle() {}
	~CDushParticle(){}

	HRESULT Init(void);
	void Update(void);

private:

};

#endif