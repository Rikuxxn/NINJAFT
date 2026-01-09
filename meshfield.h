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
		float fRadiusX;		// X方向の半径
		float fRadiusZ;		// Z方向の半径
		int nNumPrimitive;	// プリミティブ数
		int nNumIdx;		// インデックス数
		int nNumAllVtx;		// 全体頂点数
		int nNumX;			// X頂点
		int nNumZ;			// Z頂点
		int nTexIdx;		// テクスチャインデックス
	};

	//*************************************************************************
	// 川の方向
	//*************************************************************************
	enum RiverDir
	{
		RIVER_X,
		RIVER_Z
	};

	CMeshField(int nPriority = 3);
	~CMeshField();

	static CMeshField* Create(D3DXVECTOR3 pos, float fRadiusX, float fRadiusZ, int nNumX, int nNumZ);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	void CreateRiverLine(void);

	//*************************************************************************
	// flagment関数
	//*************************************************************************
	bool IsRiverArea(float worldX, float worldZ) const;
	bool IsRiverCell(float cx, float cz, float size);

	//*************************************************************************
	// setter関数
	//*************************************************************************
	void SetPos(D3DXVECTOR3 pos) { m_pos = pos; }
	void SetRaiusZ(float fRadius) { m_MeshFiled.fRadiusZ = fRadius; }
	void SetRaiusX(float fRadius) { m_MeshFiled.fRadiusX = fRadius; }

	//*************************************************************************
	// getter関数
	//*************************************************************************
	D3DXVECTOR3 GetPos(void) { return m_pos; }
	float GetRadiusX(void) { return m_MeshFiled.fRadiusX; }
	float GetRadiusZ(void) { return m_MeshFiled.fRadiusZ; }
	float GetRiverDepth(float dist) const;
	float GetRiverDistance(float worldX, float worldZ) const;
	float GetHeight(float worldX, float worldZ) const;

private:
	static constexpr float EXIT_HALF_WIDTH	= 80.0f;	// 出口ハーフ幅
	static constexpr float RIVER_WIDTH		= 65.0f;	// 川の幅
	static constexpr float RIVER_DEPTH		= 20.0f;	// 川の深さ

	LPDIRECT3DINDEXBUFFER9	m_pIdx;						// インデックスバッファ
	LPDIRECT3DVERTEXBUFFER9 m_pVtx;						// 頂点バッファ
	D3DXVECTOR3				m_pos;						// 座標
	D3DXVECTOR3				m_rot;						// 角度
	D3DXMATRIX				m_mtxWorld;					// ワールドマトリックス
	MeshField				m_MeshFiled;				// 構造体変数
	RiverDir				m_riverDir;					// 川の方向
	float					m_riverCenter;				// X or Z
	float					m_riverWidth;				// 半径
	float					m_riverDepth;				// 深さ
	std::vector<float>		m_riverLine;				// 川中心ライン
};

#endif