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
    int GetMouseOverIndex(void) const;

private:
    static constexpr float ITEM_NUM         = 3;        // 項目数
    static constexpr float ITEM_W           = 250.0f;   // 項目の幅
    static constexpr float ITEM_H           = 130.0f;   // 項目の高さ
    static constexpr float SPACING_Y        = 200.0f;   // 項目の間隔
    static constexpr float ANCHOR_X         = 0.55f;    // 横位置（%）
    static constexpr float ANCHOR_Y         = 0.35f;    // 縦位置（%）
    static constexpr float ITEM_WRATE       = 0.1f;     // 画面幅のに対しての項目幅率
    static constexpr float ITEM_HRATE       = 0.08f;    // 画面高さに対しての項目高さ率
    static constexpr float SPACING_YRATE    = 0.2f;     // 項目の間隔率

    LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff; 	            // 背景用頂点バッファへのポインタ
    std::vector<CPause*>    m_Items;                    // ポーズ項目
    int                     m_SelectedIndex;            // 選択したインデックス
    bool                    m_inputLock;                // 入力制限フラグ

};

#endif