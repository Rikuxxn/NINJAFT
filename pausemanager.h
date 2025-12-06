//=============================================================================
//
// ポーズマネージャー処理 [pausemanager.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _PAUSEMANAGER_H_// このマクロ定義がされていなかったら
#define _PAUSEMANAGER_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "pause.h"

//*****************************************************************************
// ポーズマネージャークラス
//*****************************************************************************
class CPauseManager
{
public:
    CPauseManager();
    ~CPauseManager();

    void Init(void);                    // 初期化
    void Uninit(void);                  // 終了
    void Update(void);                  // 更新（入力処理）
    void Draw(void);                    // 描画
  
private:
    LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff;	// 背景用頂点バッファへのポインタ
    std::vector<CPause*> m_Items;       // ポーズ項目
    int m_SelectedIndex;                // 選択したインデックス
    bool m_inputLock;

    int GetMouseOverIndex(void) const;
};

#endif