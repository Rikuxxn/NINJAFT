//=============================================================================
//
// メッシュの軌道処理 [meshOrbit.h]
// Author: RIKU TANEKAWA
//
//=============================================================================
#ifndef _MESHORBIT_H_
#define _MESHORBIT_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "meshbase.h"


//*****************************************************************************
// メッシュの軌道クラス
//*****************************************************************************
class CMeshOrbit : public CMeshBase
{
public:
	CMeshOrbit(int nPriority = 5);
	~CMeshOrbit();

	static CMeshOrbit* Create(const D3DXVECTOR3 top, const D3DXVECTOR3 bottom, const D3DXCOLOR col, const int nSegH);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

	//*************************************************************************
	// setter関数
	//*************************************************************************
	void SetStartPos(D3DXVECTOR3 top, D3DXVECTOR3 bottom) { m_Top = top; m_Bottom = bottom; }

private:
	D3DXVECTOR3 m_Top;		// 上の位置
	D3DXVECTOR3 m_Bottom;	// 下の位置
	D3DXCOLOR	m_col;		// 色

};

#endif

