//=============================================================================
//
// 背景処理 [background.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _BACKGROUND_H_// このマクロ定義がされていなかったら
#define _BACKGROUND_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object2D.h"

//*****************************************************************************
// 背景クラス
//*****************************************************************************
class CBackground : public CObject2D
{
public:
	CBackground();
	~CBackground();

	static CBackground* Create(D3DXVECTOR3 pos, float fWidth, float fHeight, const char* path);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	void SetPath(const char* path)
	{
		if (path == nullptr)
		{
			path = " ";
		}

		strcpy_s(m_szPath, MAX_PATH, path); 
	}

private:
	int		m_nIdxTexture;		// テクスチャインデックス
	char	m_szPath[MAX_PATH];	// ファイルパス

};

#endif
