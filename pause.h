//=============================================================================
//
// ポーズ処理 [pause.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _PAUSE_H_// このマクロ定義がされていなかったら
#define _PAUSE_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object2D.h"


//*****************************************************************************
// ポーズクラス
//*****************************************************************************
class CPause : public CObject2D
{
public:
	CPause(int nPriority = 7);
	~CPause();

	// 選択項目の種類
	typedef enum
	{
		MENU_CONTINUE = 0,		// ゲームに戻る
		MENU_RETRY,				// ゲームをやり直す
		MENU_QUIT,				// タイトル画面に戻る
		MENU_MAX
	}MENU;

	static CPause* Create(MENU type, D3DXVECTOR3 pos,float fWidth,float fHeight);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	bool IsMouseOver(void);
	virtual void Execute(void) {};

	void SetPath(const char* path) { strcpy_s(m_szPath, MAX_PATH, path); }

	// 選択状態設定・取得
	void SetSelected(bool selected) { m_isSelected = selected; }
	bool IsSelected(void) const { return m_isSelected; }

private:
	int m_nIdxTexture;						// テクスチャインデックス
	char m_szPath[MAX_PATH];				// ファイルパス
	bool m_isSelected;						// 選んだか
};

//*****************************************************************************
// コンティニュー項目クラス
//*****************************************************************************
class CContinue : public CPause
{
public:
	CContinue();
	~CContinue();

	void Execute(void) override;

private:

};

//*****************************************************************************
// リトライ項目クラス
//*****************************************************************************
class CRetry : public CPause
{
public:
	CRetry();
	~CRetry();

	void Execute(void) override;

private:

};

//*****************************************************************************
// 終了項目クラス
//*****************************************************************************
class CQuit : public CPause
{
public:
	CQuit();
	~CQuit();

	void Execute(void) override;

private:

};

#endif
