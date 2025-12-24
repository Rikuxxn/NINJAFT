//=============================================================================
//
// ビルボード処理 [objectBillboard.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _OBJECTBILLBOARD_H_// このマクロ定義がされていなかったら
#define _OBJECTBILLBOARD_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"


//*****************************************************************************
// ビルボードクラス
//*****************************************************************************
class CObjectBillboard : public CObject
{
public:
	CObjectBillboard(int nPriority = 5);
	~CObjectBillboard();

	//*************************************************************************
	// ビルボード生成テンプレート
	//*************************************************************************
	// billboardType : 生成したいビルボードクラス名
	template <typename billboardType>
	static CObjectBillboard* Create(const char* path, D3DXVECTOR3 pos, float fWidth, float fHeight)
	{
		// 渡された型がCObjectBillboardを継承していなかった場合
		static_assert(std::is_base_of<CObjectBillboard, billboardType>::value, "型はCObjectBillboardを継承していません。");

		billboardType* pBillboard = new billboardType();// 型のインスタンス生成

		if (!pBillboard)
		{
			return nullptr;
		}

		pBillboard->SetPath(path);
		pBillboard->m_pos = pos;
		pBillboard->m_fSize = fWidth;// エフェクトの半径
		pBillboard->SetHeight(fHeight);
		pBillboard->m_col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

		// 初期化処理
		pBillboard->Init();

		return pBillboard;
	}

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void UpdateTurn(void);
	void Draw(void);
	void UpdateGuageVtx(float fRate);
	void UpdateFrame(void);

	void SetPos(D3DXVECTOR3 pos) { m_pos = pos; }
	void SetRot(D3DXVECTOR3 rot) { m_rot = rot; }
	void SetCol(D3DXCOLOR col) { m_col = col; }
	void SetSize(float fRadius) { m_fSize = fRadius; }
	void SetSize(float fWidth, float fHeight) { m_fWidth = fWidth; m_fHeight = fHeight; }
	void SetPath(const char* path)
	{ 
		if (path == nullptr)
		{
			path = " ";
		}

		strcpy_s(m_szPath, MAX_PATH, path); 
	}

	D3DXVECTOR3 GetPos(void) { return m_pos; }
	D3DXVECTOR3 GetRot(void) { return m_rot; }
	D3DXCOLOR GetCol(void) { return m_col; }

private:
	LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff;			// 頂点バッファへのポインタ
	D3DXVECTOR3				m_pos;				// 位置
	D3DXVECTOR3				m_rot;				// 向き
	D3DXCOLOR				m_col;				// 色
	D3DXMATRIX				m_mtxWorld;			// ワールドマトリックス
	float					m_fSize;			// サイズ(エフェクト半径)
	float					m_fWidth;			// サイズ
	float					m_fHeight;			// サイズ(ビルボード)
	int						m_nIdxTexture;		// テクスチャインデックス
	char					m_szPath[MAX_PATH];	// ファイルパス
};

#endif