//=============================================================================
//
// メッシュシリンダー処理 [meshcylinder.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _MESHCYLINDER_H_
#define _MESHCYLINDER_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"


//*****************************************************************************
// メッシュシリンダークラス
//*****************************************************************************
class CMeshCylinder : public CObject
{
public:
	CMeshCylinder(int nPriority = 5);
	~CMeshCylinder();

	static CMeshCylinder* Create(D3DXVECTOR3 pos, D3DXCOLOR col, float fRad, float fWeight, float incRad, int nLife, float decAlpha);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

	D3DXVECTOR3 GetPos(void) { return m_pos; }
	D3DXCOLOR GetCol(void) { return m_col; }

	void SetPos(D3DXVECTOR3 pos) { m_pos = pos; }
	void SetCol(D3DXCOLOR col) { m_col = col; }
	void SetRadius(float nRad) { m_fRadius = nRad; }
	void SetIncRadius(float incRad) { m_incRadius = incRad; }
	void SetLife(int nLife) { m_nLife = nLife; m_nMaxLife = nLife; }
	void SetDecAlpha(float alpha) { m_decAlpha = alpha; }
	void SetLineweight(float val) { m_fLineweight = val; }
private:
	static constexpr float	MESHCYLINDER_HEIGHT		= 20.0f;	// 縦幅
	static constexpr int	MESHCYLINDER_X			= 20;		// X方向のブロック数
	static constexpr int	MESHCYLINDER_Z			= 1;		// Z方向のブロック数
	static constexpr int	MESHCYLINDER_V			= 1;		// 垂直方向の分割数
	static constexpr int	MESHCYLINDER_H			= 20;		// 水平方向の分割数
	static constexpr int	MESHCYLINDER_PRIMITIVE	= (MESHCYLINDER_X * MESHCYLINDER_Z) * 2 + (4 * (MESHCYLINDER_Z - 1));// プリミティブ数
	static constexpr int	MESHCYLINDER_VERTEX		= (MESHCYLINDER_X + 1) * (MESHCYLINDER_Z + 1);// 頂点数
	static constexpr int	MESHCYLINDER_INDEX		= MESHCYLINDER_PRIMITIVE + 2;// インデックス数

	LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff;		// 頂点バッファへのポインタ
	LPDIRECT3DINDEXBUFFER9	m_pIdxBuff;		// インデックスバッファへのポインタ
	D3DXMATRIX				m_mtxWorld;		// ワールドマトリックス
	D3DXVECTOR3				m_pos;			// 位置
	D3DXVECTOR3				m_rot;			// 向き
	D3DXCOLOR				m_col;			// 色
	int						m_nLife;		// 現在の寿命
	int						m_nMaxLife;		// 設定時の寿命
	float					m_fRadius;		// 半径
	float					m_fLineweight;	// 太さ
	float					m_incRadius;	// 半径の増加量
	float					m_decAlpha;		// アルファ値の減少量
};

#endif
