//=============================================================================
//
// 項目選択処理 [itemselect.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _ITEMSELECT_H_// このマクロ定義がされていなかったら
#define _ITEMSELECT_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "item.h"

//*****************************************************************************
// 項目選択クラス
//*****************************************************************************
class CItemSelect
{
public:
    CItemSelect();
    ~CItemSelect();

    void Init(void);                    // 初期化
    void Uninit(void);
    void Update(void);                  // 更新（入力処理）
    void Draw(void);                    // 描画

    static void SetSelectedItem(int id) { m_SelectedIndex = id; }
    static int GetSelectedItem(void) { return m_SelectedIndex; }

private:
    int GetMouseOverIndex(void) const;

private:
    static constexpr float ITEM_NUM         = 2;        // 項目数
    static constexpr float ITEM_W           = 150.0f;   // 項目の幅
    static constexpr float ITEM_H           = 60.0f;    // 項目の高さ
    static constexpr float SPACING_Y        = 140.0f;   // 項目の間隔
    static constexpr float ANCHOR_X         = 0.3f;     // 横位置（%）
    static constexpr float ANCHOR_Y         = 0.6f;     // 縦位置（%）
    static constexpr float ITEM_WRATE       = 0.09f;    // 画面幅のに対しての項目幅率
    static constexpr float ITEM_HRATE       = 0.06f;    // 画面高さに対しての項目高さ率


    std::vector<CItem*> m_Item;             // 項目
    static int          m_SelectedIndex;    // 選択したインデックス
    bool                m_inputLock;        // 入力制限フラグ

};

#endif

