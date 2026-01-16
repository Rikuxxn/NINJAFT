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

	// UIレイアウト構造体
	struct UILayout
	{
		float anchorX;   // 0.0～1.0
		float anchorY;   // 0.0～1.0
		float widthRate; // 画面比率
		float heightRate;
	};

	static CBackground* Create(const char* path, float anchorX, float anchorY, float widthRate, float heightRate);
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
	void SetAnchor(float x, float y)
	{
		m_layout.anchorX = x;
		m_layout.anchorY = y;
	}
	void SetSizeRate(float width, float height) { m_layout.widthRate = width; m_layout.heightRate = height; }

private:
	int			m_nIdxTexture;		// テクスチャインデックス
	char		m_szPath[MAX_PATH];	// ファイルパス
	UILayout    m_layout;           // レイアウト構造体変数

};

#endif
