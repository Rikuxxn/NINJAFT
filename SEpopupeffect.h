//=============================================================================
//
// 効果音ポップアップエフェクト処理 [SEpopupeffect.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _SE_POPUPEFFECT_H_// このマクロ定義がされていなかったら
#define _SE_POPUPEFFECT_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "effect.h"


//*****************************************************************************
// 効果音ポップアップエフェクトクラス
//*****************************************************************************
class CSEPopupEffect : public CEffect
{
public:
	CSEPopupEffect();
	~CSEPopupEffect();

	static CSEPopupEffect* Create(const char* path, D3DXVECTOR3 pos, D3DXCOLOR col, int nLife);
	HRESULT Init(void);
	void Update(void);
	void SetupParameter(void);
	void SetLife(int nLife) { m_nLife = nLife; }
	void SetPath(const char* path)
	{ 
		// nullptrだったら
		if (path == nullptr)
		{
			path = " ";
		}

		strcpy_s(m_szPath, MAX_PATH, path); 
	}

	int GetLife(void) { return m_nLife; }

private:
	char	m_szPath[MAX_PATH];	// ファイルパス
	int		m_nLife;			// 寿命

};

#endif
