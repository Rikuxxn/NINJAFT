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
		float	fRadiusX;			// X方向の半径
		float	fRadiusZ;			// Z方向の半径
		int		nNumPrimitive;		// プリミティブ数
		int		nNumIdx;			// インデックス数
		int		nNumAllVtx;			// 全体頂点数
		int		nNumX;				// X頂点
		int		nNumZ;				// Z頂点
		int		nTexIdx;			// テクスチャインデックス
		float	fSurfaceHeight;		// 水面高さ
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
	void SetFlowDir(const D3DXVECTOR3& dir)
	{
		m_FlowDir = dir;

		// 正規化する
		D3DXVec3Normalize(&m_FlowDir, &m_FlowDir);
	}

	//*************************************************************************
	// getter関数
	//*************************************************************************
	D3DXVECTOR3 GetPos(void) { return m_pos; }
	float GetRadiusX(void) { return m_WaterFiled.fRadiusX; }
	float GetRadiusZ(void) { return m_WaterFiled.fRadiusZ; }

private:
	static constexpr int	SPAWN_TIME		= 7;		// 生成間隔(フレーム)
	static constexpr float	SPAWN_RADIUS	= 10.0f;	// 生成半径
	static constexpr float	UV_RATE_X		= 5.0f;		// テクスチャ座標の割合X
	static constexpr float	UV_RATE_Y		= 5.0f;		// テクスチャ座標の割合Y
	static constexpr float	OFFSET_POS		= 10.0f;	// 波紋生成オフセット位置
	static constexpr float	FLOW_SPEED		= 0.002f;	// 川の流れる速度

	LPDIRECT3DINDEXBUFFER9	m_pIdx;						// インデックスバッファ
	LPDIRECT3DVERTEXBUFFER9 m_pVtx;						// 頂点バッファ
	D3DXVECTOR3				m_pos;						// 座標
	D3DXVECTOR3				m_rot;						// 角度
	D3DXMATRIX				m_mtxWorld;					// ワールドマトリックス
	WaterField				m_WaterFiled;				// 構造体変数
	float					m_UVOffsetU;				// UVのオフセット(U)
	float					m_UVOffsetV;				// UVのオフセット(V)
	D3DXVECTOR3				m_FlowDir;					// 川の流れる方向
	float					m_FlowSpeed;				// 川の流れる速度
};

#endif