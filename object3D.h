//=============================================================================
//
// 3Dオブジェクト処理 [object3D.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _OBJECT3D_H_// このマクロ定義がされていなかったら
#define _OBJECT3D_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"


//*****************************************************************************
// 3Dオブジェクトクラス
//*****************************************************************************
class CObject3D : public CObject
{
public:
	CObject3D(int nPriority = 5);
	~CObject3D();

	static CObject3D* Create(void);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

	void SetPos(D3DXVECTOR3 pos) { m_pos = pos; }
	void SetRot(D3DXVECTOR3 rot) { m_rot = rot; }
	void SetWidth(float fWidth) { m_fWidth = fWidth; }
	void SetHeight(float fHeight) { m_fHeight = fHeight; }
	void SetCol(D3DXCOLOR col) { m_col = col; }

	D3DXVECTOR3 GetPos(void) { return m_pos; }
	D3DXVECTOR3 GetRot(void) { return m_rot; }

private:
	LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff;		// 頂点バッファへのポインタ
	D3DXVECTOR3 m_pos;						// 位置
	D3DXVECTOR3 m_rot;						// 向き
	D3DXCOLOR m_col;						// 色
	D3DXMATRIX m_mtxWorld;					// ワールドマトリックス
	float m_fWidth;							// 幅
	float m_fHeight;						// 高さ
};

#endif
