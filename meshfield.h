//=============================================================================
//
// メッシュフィールド処理 [ meshfield.h ]
// Author: RIKU TANEKAWA
//
//=============================================================================
#ifndef _MESHFIELD_H_
#define _MESHFIELD_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"

//*****************************************************************************
// メッシュフィールドクラス
//*****************************************************************************
class CMeshField : public CObject
{
public:

	//*************************************************************************
	// 構造体宣言
	//*************************************************************************
	struct MeshField
	{
		float fRadius;		// X方向の半径
		float fRadiusZ;		// Z方向の半径
		int nNumPrimitive;	// プリミティブ数
		int nNumIdx;		// インデックス数
		int nNumAllVtx;		// 全体頂点数
		int nNumX;			// X頂点
		int nNumZ;			// Z頂点
		int nTexIdx;		// テクスチャインデックス
	};

	CMeshField(int nPriority = 5);
	~CMeshField();

	static CMeshField* Create(D3DXVECTOR3 pos, float fRadiusX, float fRadiusZ, int nNumX, int nNumZ);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

	//*************************************************************************
	// setter関数
	//*************************************************************************
	void SetPos(D3DXVECTOR3 pos) { m_pos = pos; }
	void SetRaiusZ(float fRadius) { m_MeshFiled.fRadiusZ = fRadius; }
	void SetRaiusX(float fRadius) { m_MeshFiled.fRadius = fRadius; }

	//*************************************************************************
	// getter関数
	//*************************************************************************
	D3DXVECTOR3 GetPos(void) { return m_pos; }
	float GetRadiusX(void) { return m_MeshFiled.fRadius; }
	float GetRadiusZ(void) { return m_MeshFiled.fRadiusZ; }

private:

	LPDIRECT3DINDEXBUFFER9	m_pIdx;		// インデックスバッファ
	LPDIRECT3DVERTEXBUFFER9 m_pVtx;		// 頂点バッファ
	D3DXVECTOR3				m_pos;		// 座標
	D3DXVECTOR3				m_rot;		// 角度
	D3DXMATRIX				m_mtxWorld;	// ワールドマトリックス
	MeshField				m_MeshFiled;// 構造体変数
};

#endif