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
    std::vector<CItem*> m_Item;             // 項目
    static int          m_SelectedIndex;    // 選択したインデックス
    bool                m_inputLock;        // 入力制限フラグ

};

#endif

