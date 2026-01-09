//=============================================================================
//
// 水フィールド処理 [ waterfield.h ]
// Author: RIKU TANEKAWA
//
//=============================================================================
#ifndef _WATERFIELD_H_
#define _WATERFIELD_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"

//*****************************************************************************
// 水フィールドクラス
//*****************************************************************************
class CWaterField : public CObject
{
public:

	//*************************************************************************
	// 構造体宣言
	//*************************************************************************
	struct WaterField
	{
		float fRadiusX;			// X方向の半径
		float fRadiusZ;			// Z方向の半径
		int nNumPrimitive;		// プリミティブ数
		int nNumIdx;			// インデックス数
		int nNumAllVtx;			// 全体頂点数
		int nNumX;				// X頂点
		int nNumZ;				// Z頂点
		int nTexIdx;			// テクスチャインデックス
		float fSurfaceHeight;	// 水面高さ
	};

	CWaterField(int nPriority = 4);
	~CWaterField();

	static CWaterField* Create(D3DXVECTOR3 pos, float fRadiusX, float fRadiusZ, int nNumX, int nNumZ);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	void SpawnCylinder(void);

	//*************************************************************************
	// flagment関数
	//*************************************************************************
	bool IsInWater(const D3DXVECTOR3& pos);

	//*************************************************************************
	// setter関数
	//*************************************************************************
	void SetPos(D3DXVECTOR3 pos) { m_pos = pos; }
	void SetRaiusZ(float fRadius) { m_WaterFiled.fRadiusZ = fRadius; }
	void SetRaiusX(float fRadius) { m_WaterFiled.fRadiusX = fRadius; }

	//*************************************************************************
	// getter関数
	//*************************************************************************
	D3DXVECTOR3 GetPos(void) { return m_pos; }
	float GetRadiusX(void) { return m_WaterFiled.fRadiusX; }
	float GetRadiusZ(void) { return m_WaterFiled.fRadiusZ; }

private:
	static constexpr int	SPAWN_TIME		= 7;		// 生成間隔(フレーム)
	static constexpr float	SPAWN_RADIUS	= 10.0f;	// 生成半径

	LPDIRECT3DINDEXBUFFER9	m_pIdx;			// インデックスバッファ
	LPDIRECT3DVERTEXBUFFER9 m_pVtx;			// 頂点バッファ
	D3DXVECTOR3				m_pos;			// 座標
	D3DXVECTOR3				m_rot;			// 角度
	D3DXMATRIX				m_mtxWorld;		// ワールドマトリックス
	WaterField				m_WaterFiled;	// 構造体変数
};

#endif