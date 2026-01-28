//=============================================================================
//
// メッシュのベース処理 [meshbase.h]
// Author: RIKU TANEKAWA
//
//=============================================================================
#ifndef _MESHBASE_H_
#define _MESHBASE_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"


//*****************************************************************************
// メッシュベースクラス
//*****************************************************************************
class CMeshBase : public CObject
{
public:
	CMeshBase(int nPriority = 3);
	~CMeshBase();

	virtual HRESULT Init(void);
	virtual void Uninit(void);
	virtual void Draw(void);

	//*************************************************************************
	// setter関数
	//*************************************************************************
	void SetPosition(const D3DXVECTOR3 pos) { m_pos = pos; }
	void SetRotation(const D3DXVECTOR3 rot) { m_rot = rot; }
	void UpdatePosition(const D3DXVECTOR3 speed) { m_pos += speed; }
	void UpdateRotation(const D3DXVECTOR3 speed) { m_rot += speed; }
	void SetVtxBuffer(const D3DXVECTOR3 pos, const int nIdx, const D3DXVECTOR2 tex, const D3DXVECTOR3 nor, const D3DXCOLOR col);
	void SetIndexBuffer(const WORD Idx, const int nCnt);
	void SetTexPath(const char* filepath);
	void SetVtxElement(const int vertex, const int polygon, const int index);
	void SetSegment(const int nSegH, const int nSegV) { m_nSegmentH = nSegH; m_nSegmentV = nSegV; }
	void SetMatrix(void);
	void SetVtxPos(const D3DXVECTOR3 pos, const int nIdx);
	void SetNormal(const D3DXVECTOR3 nor, const int nIdx);
	void SetVtxColor(const D3DXCOLOR col, const int nIdx);
	void SetTexture(const D3DXVECTOR2 tex, const int nIdx);

	//*************************************************************************
	// getter関数
	//*************************************************************************
	int GetSegH(void) const { return m_nSegmentH; }
	int GetSegV(void) const { return m_nSegmentV; }
	D3DXVECTOR3 GetPosition(void) const { return m_pos; }
	D3DXVECTOR3 NormalizeNormal(const int nIdx);
	D3DXVECTOR3 GetVtxPos(const int nIdx);
	int GetIndex(const int nIdx);
	D3DXCOLOR GetColor(const int nIdx);

private:
	LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff;			// 頂点バッファへのポインタ
	LPDIRECT3DINDEXBUFFER9	m_pIdxBuff;			// インデックスバッファへのポインタ
	D3DXVECTOR3				m_pos;				// 位置
	D3DXVECTOR3				m_rot;				// 向き
	D3DXMATRIX				m_mtxWorld;			// ワールドマトリックス
	int						m_nSegmentH;		// 横の分割数
	int						m_nSegmentV;		// 縦の分割数
	int						m_nIdxTexture;		// テクスチャのインデックス
	int						m_nNumVtx;			// 頂点数
	int						m_nNumPolygon;		// ポリゴン数
	int						m_nNumIdx;			// インデックスバッファ
};

#endif

