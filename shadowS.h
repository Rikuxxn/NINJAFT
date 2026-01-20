//=============================================================================
//
// ステンシルシャドウ処理 [shadowS.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _SHADOWS_H_// このマクロ定義がされていなかったら
#define _SHADOWS_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "objectX.h"

//*****************************************************************************
// ステンシルシャドウクラス
//*****************************************************************************
class CShadowS : public CObjectX
{
public:
	CShadowS(int nPriority = 4);
	~CShadowS();

	static CShadowS* Create(const char* pFilepath, D3DXVECTOR3 size);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	void SetPosition(D3DXVECTOR3 pos) { m_pos = pos; }
	void SetStencilRef(DWORD value) { m_stencilRef = value; }
private:
	LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff;		// 頂点バッファへのポインタ
	D3DXVECTOR3				m_pos;			// 2Dポリゴン描画用位置
	D3DCOLOR				m_col;			// 2Dポリゴン描画用色
	float					m_fWidth;		// 幅(画面サイズの2Dポリゴン用)
	float					m_fHeight;		// 高さ(画面サイズの2Dポリゴン用)
	DWORD					m_stencilRef;	// ステンシルバッファの値

};
#endif
