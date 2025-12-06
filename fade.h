//=============================================================================
//
// フェード処理 [fade.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _FADE_H_// このマクロ定義がされていなかったら
#define _FADE_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object2D.h"
#include "scene.h"

//*****************************************************************************
// フェードクラス
//*****************************************************************************
class CFade
{
public:
	// 状態
	typedef enum
	{
		FADE_NONE = 0,    // 何もしていない状態
		FADE_IN,          // フェードイン状態
		FADE_OUT,         // フェードアウト状態
		FADE_MAX
	}FADE;

	CFade();
	~CFade();

	static CFade* Create(CScene::MODE mode);
	HRESULT Init(CScene::MODE mode);
	void Uninit(void);
	void Update(void);
	void Draw(void);

	void SetFade(CScene::MODE modeNext);
	FADE GetFade(void) { return m_fade; }

private:
	LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff;	// 頂点バッファへのポインタ
	D3DXVECTOR3 m_pos;					// 位置
	FADE m_fade;						// フェードの状態
	D3DXCOLOR m_fadeCol;				// フェードの色
	CScene::MODE m_SceneNext;			// 次の画面
	float m_fWidth;						// 幅
	float m_fHeight;					// 高さ
};

#endif