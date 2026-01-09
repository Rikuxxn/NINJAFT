//=============================================================================
//
// モデル処理 [model.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _MODEL_H_// このマクロ定義がされていなかったら
#define _MODEL_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"
#include "charactermanager.h"

//*****************************************************************************
// モデルクラス
//*****************************************************************************
class CModel
{
public:
	CModel();
	~CModel();

	static CModel* Create(const char* pFilepath, D3DXVECTOR3 pos, D3DXVECTOR3 rot);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	void DrawNormal(LPDIRECT3DDEVICE9 pDevice);

	//*****************************************************************************
	// setter関数
	//*****************************************************************************
	void SetOutlineShaderConstants(LPDIRECT3DDEVICE9 pDevice);
	void SetPos(D3DXVECTOR3 pos) { m_pos = pos; }
	void SetRot(D3DXVECTOR3 rot) { m_rot = D3DXToRadian(rot); }
	void SetCol(D3DXCOLOR col) { m_col = col; }
	void SetParent(CModel* pModel) { m_pParent = pModel; }
	void SetOffsetPos(const D3DXVECTOR3& pos) { m_OffsetPos = pos; }
	void SetOffsetRot(const D3DXVECTOR3& rot) { m_OffsetRot = rot; }
	void SetOutlineColor(const D3DXVECTOR4& col) { m_outlineColor = col; }

	//*****************************************************************************
	// getter関数
	//*****************************************************************************
	D3DXMATRIX GetMtxWorld(void) { return m_mtxWorld; }
	D3DXVECTOR3 GetPos(void) { return m_pos; }
	D3DXVECTOR3 GetRot(void) { return m_rot; }
	D3DXCOLOR GetCol(void) { return m_col; }
	D3DXVECTOR3 GetOffsetPos(void) const { return m_OffsetPos; }
	D3DXVECTOR3 GetOffsetRot(void) const { return m_OffsetRot; }
	const char* GetPath(void) { return m_Path; }
	CModel* GetParent(void) { return m_pParent; }

private:
	int*					m_nIdxTexture;			// テクスチャインデックス
	D3DXVECTOR3				m_pos;					// 位置
	D3DXVECTOR3				m_rot;					// 向き
	D3DXCOLOR				m_col;					// 色
	D3DXVECTOR3				m_move;					// 移動量
	LPD3DXMESH				m_pMesh;				// メッシュへのポインタ
	LPD3DXBUFFER			m_pBuffMat;				// マテリアルへのポインタ
	DWORD					m_dwNumMat;				// マテリアル数
	D3DXMATRIX				m_mtxWorld;				// ワールドマトリックス
	CModel*					m_pParent;				// 親モデルへのポインタ
	char					m_Path[MAX_PATH];		// ファイルパス
	D3DXVECTOR3				m_OffsetPos;			// オフセット
	D3DXVECTOR3				m_OffsetRot;			// オフセット
	LPDIRECT3DVERTEXSHADER9 m_pOutlineVS;			// 頂点シェーダ
	LPDIRECT3DPIXELSHADER9  m_pOutlinePS;			// ピクセルシェーダ
	LPD3DXCONSTANTTABLE     m_pVSConsts;			// 頂点シェーダコンスタントテーブル
	LPD3DXCONSTANTTABLE     m_pPSConsts;			// ピクセルシェーダコンスタントテーブル
	D3DXVECTOR4				m_outlineColor;			// アウトラインカラー
};

#endif
