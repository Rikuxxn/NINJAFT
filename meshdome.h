//=============================================================================
//
// メッシュドーム処理 [meshdome.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _MESHDOME_H_
#define _MESHDOME_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"


//*****************************************************************************
// メッシュドームクラス
//*****************************************************************************
class CMeshDome : public CObject
{
public:
	CMeshDome(int nPriority = 5);
	~CMeshDome();

	static CMeshDome* Create(D3DXVECTOR3 pos, int nRadius);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

	D3DXVECTOR3 GetPos(void) { return m_pos; }

private:
	static constexpr int MESHDOME_X			= 18;														// Xブロック数
	static constexpr int MESHDOME_Z			= 18;														// Zブロック数
	static constexpr int MESHDOME_VERTEX	= (MESHDOME_X + 1) * (MESHDOME_Z + 1);						// 頂点数
	static constexpr int MESHDOME_INDEX		= MESHDOME_X * MESHDOME_Z * 6;								// インデックス数
	static constexpr int MESHDOME_PRIMITIVE = (MESHDOME_X * MESHDOME_Z * 2) + ((MESHDOME_Z - 1) * 2);	// ポリゴン数

	LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff;		// 頂点バッファへのポインタ
	LPDIRECT3DINDEXBUFFER9	m_pIdxBuff;		// インデックスバッファへのポインタ
	D3DXMATRIX				m_mtxWorld;		// ワールドマトリックス
	D3DXVECTOR3				m_pos;			// 位置
	D3DXVECTOR3				m_rot;			// 向き
	D3DXCOLOR				m_col;			// 色
	int						m_nRadius;		// 半径
	int						m_nIdxTexture;	// テクスチャインデックス
};

#endif

