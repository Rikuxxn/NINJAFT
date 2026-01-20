//=============================================================================
//
// Xファイル処理 [objectX.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _OBJECTX_H_// このマクロ定義がされていなかったら
#define _OBJECTX_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"

//*****************************************************************************
// Xファイルクラス
//*****************************************************************************
class CObjectX : public CObject
{
public:
	CObjectX(int nPriority = 3);
	virtual ~CObjectX();

	static CObjectX* Create(const char* pFilepath, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 size);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

	void SetSize(D3DXVECTOR3 size) { m_size = size; }
	void SetPos(D3DXVECTOR3 pos) { m_pos = pos; }
	void SetRot(D3DXVECTOR3 rot) { m_rot = rot; }
	void SetGhostObject(bool flag) { m_isGhostObject = flag; }
	void SetPath(const char* path)
	{
		// nullptrだったら
		if (path == nullptr)
		{
			path = " ";
		}

		strcpy_s(m_szPath, MAX_PATH, path);
	}

	const char* GetPath(void) { return m_szPath; }
	D3DXVECTOR3 GetPos(void) { return m_pos; }
	D3DXVECTOR3 GetRot(void) { return m_rot; }
	D3DXVECTOR3 GetSize(void) const { return m_size; }		// 拡大率
	D3DXVECTOR3 GetModelSize(void) { return m_modelSize; }	// モデルの元サイズ
	virtual D3DXCOLOR GetCol(void) const { return D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f); }
	D3DXCOLOR GetMaterialColor(void) const;
	bool IsGhostObject(void) const { return m_isGhostObject; }

private:
	int*			m_nIdxTexture;
	D3DXVECTOR3		m_pos;				// 位置
	D3DXVECTOR3		m_rot;				// 向き
	D3DXVECTOR3		m_move;				// 移動量
	D3DXVECTOR3		m_size;				// サイズ
	LPD3DXMESH		m_pMesh;			// メッシュへのポインタ
	LPD3DXBUFFER	m_pBuffMat;			// マテリアルへのポインタ
	DWORD			m_dwNumMat;			// マテリアル数
	D3DXMATRIX		m_mtxWorld;			// ワールドマトリックス
	char			m_szPath[MAX_PATH];	// ファイルパス
	D3DXVECTOR3		m_modelSize;		// モデルの元サイズ（全体の幅・高さ・奥行き）
	bool			m_isGhostObject;	// ゴーストオブジェクト(透明)かどうか
};

#endif