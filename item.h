//=============================================================================
//
// 項目処理 [item.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _ITEM_H_// このマクロ定義がされていなかったら
#define _ITEM_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object2D.h"

//*****************************************************************************
// 項目クラス
//*****************************************************************************
class CItem : public CObject2D
{
public:
	CItem(int nPriority = 7);
	~CItem();

	// 選択項目の種類
	typedef enum
	{
		ITEM_ID_PLAY = 0,
		ITEM_ID_EXIT,
		ITEM_MAX
	}ITEM;

	static CItem* Create(ITEM type, D3DXVECTOR3 pos, float fWidth, float fHeight);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	virtual void Execute(void) {};

	bool IsMouseOver(void);
	void SetPath(const char* path)
	{
		if (path == nullptr)
		{
			path = " ";
		}

		strcpy_s(m_szPath, MAX_PATH, path); 
	}

	// 選択状態設定・取得
	void SetSelected(bool selected) { m_isSelected = selected; }
	bool IsSelected(void) const { return m_isSelected; }
	ITEM GetItemId(void) const;

private:
	int		m_nIdxTexture;				// テクスチャインデックス
	char	m_szPath[MAX_PATH];			// ファイルパス
	bool	m_isSelected;				// 選択したか

};

//*****************************************************************************
// プレイ項目クラス
//*****************************************************************************
class CPlay : public CItem
{
public:
	CPlay();
	~CPlay();

	void Execute(void) override;

private:

};

//*****************************************************************************
// やめる項目クラス
//*****************************************************************************
class CExit : public CItem
{
public:
	CExit();
	~CExit();

	void Execute(void) override;

private:

};

#endif

