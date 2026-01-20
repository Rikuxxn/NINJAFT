//=============================================================================
//
// 項目選択処理 [itemselect.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "itemselect.h"
#include "manager.h"

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
int CItemSelect::m_SelectedIndex = 0;


//=============================================================================
// コンストラクタ
//=============================================================================
CItemSelect::CItemSelect()
{
    // 値のクリア
    m_inputLock = false;
}
//=============================================================================
// デストラクタ
//=============================================================================
CItemSelect::~CItemSelect()
{
    // なし
}
//=============================================================================
// 初期化処理
//=============================================================================
void CItemSelect::Init(void)
{
    // 空にする
    m_Item.clear();

    // 仮の初期位置（Updateで上書き）
    for (int nCnt = 0; nCnt < ITEM_NUM; nCnt++)
    {
        CItem* item = nullptr;

        switch (nCnt)
        {
        case CItem::ITEM_ID_PLAY:
            item = CItem::Create(CItem::ITEM_ID_PLAY, {}, ITEM_W, ITEM_H);
            break;
        case CItem::ITEM_ID_EXIT:
            item = CItem::Create(CItem::ITEM_ID_EXIT, {}, ITEM_W, ITEM_H);
            break;
        }

        item->Init();
        item->SetSelected(false);
        m_Item.push_back(item);
    }

    m_SelectedIndex = 0;
    m_Item[m_SelectedIndex]->SetSelected(true);
    m_inputLock = false;
}
//=============================================================================
// 終了処理
//=============================================================================
void CItemSelect::Uninit(void)
{
    // 空にする
    m_Item.clear();
}
//=============================================================================
// 更新処理
//=============================================================================
void CItemSelect::Update(void)
{
    // バックバッファサイズの取得
    float screenW = (float)CManager::GetRenderer()->GetBackBufferWidth();
    float screenH = (float)CManager::GetRenderer()->GetBackBufferHeight();

    // 横にずらす
    float baseX = screenW * ANCHOR_X;
    float startY = screenH * ANCHOR_Y;

    // サイズも画面サイズに伴って調整
    float itemW = screenW * ITEM_WRATE;
    float itemH = screenH * ITEM_HRATE;

    // 項目の位置更新
    for (int nCnt = 0; nCnt < (int)m_Item.size(); nCnt++)
    {
        m_Item[nCnt]->SetSize(itemW, itemH);

        D3DXVECTOR3 pos(
            baseX - itemW * 0.5f,
            startY + nCnt * SPACING_Y,
            0.0f
        );

        m_Item[nCnt]->SetPos(pos);
    }

    int mouseOver = GetMouseOverIndex();

    // マウスオーバー時の選択変更＆SE
    if (mouseOver != -1 && mouseOver != m_SelectedIndex && CManager::GetFade()->GetFade() == CFade::FADE_NONE)
    {
        m_SelectedIndex = mouseOver;

        // 選択SE
        CManager::GetSound()->Play(CSound::SOUND_LABEL_SELECT);
    }

    // ゲームパッドでの上下移動
    bool up = CManager::GetInputJoypad()->GetTrigger(CInputJoypad::JOYKEY_UP);
    bool down = CManager::GetInputJoypad()->GetTrigger(CInputJoypad::JOYKEY_DOWN);

    if ((up || down) && !m_inputLock && CManager::GetFade()->GetFade() == CFade::FADE_NONE)
    {
        // 選択SE
        CManager::GetSound()->Play(CSound::SOUND_LABEL_SELECT);

        if (up)
        {
            m_SelectedIndex--;
        }
        else
        {
            m_SelectedIndex++;
        }

        if (m_SelectedIndex < 0)
        {
            m_SelectedIndex = (int)m_Item.size() - 1;
        }
        if (m_SelectedIndex >= (int)m_Item.size())
        {
            m_SelectedIndex = 0;
        }

        m_inputLock = true;
    }

    if (!up && !down)
    {
        m_inputLock = false;
    }

    // クリックまたはゲームパッドボタンで実行
    if (CManager::GetFade()->GetFade() == CFade::FADE_NONE)
    {
        bool confirm = false;

        // マウスクリック
        if (mouseOver != -1 && CManager::GetInputMouse()->GetTrigger(0))
        {
            confirm = true;
        }

        // ゲームパッド
        if (CManager::GetInputJoypad()->GetTrigger(CInputJoypad::JOYKEY_A))
        {
            confirm = true;
        }

        if (confirm)
        {
            // 決定SE
            CManager::GetSound()->Play(CSound::SOUND_LABEL_ENTER);

            // 選ばれたステージIDを保存
            SetSelectedItem(m_Item[m_SelectedIndex]->GetItemId());

            // 各項目の選択時処理
            m_Item[m_SelectedIndex]->Execute();
        }
    }

    // 選択状態更新
    for (size_t nCnt = 0; nCnt < m_Item.size(); nCnt++)
    {
        m_Item[nCnt]->SetSelected(nCnt == static_cast<size_t>(m_SelectedIndex));

        // ステージの更新処理
        m_Item[nCnt]->Update();
    }
}
//=============================================================================
// 描画処理
//=============================================================================
void CItemSelect::Draw(void)
{
    for (auto item : m_Item)
    {
        if (item)
        {
            item->Draw();
        }
    }
}
//=============================================================================
// マウス選択処理
//=============================================================================
int CItemSelect::GetMouseOverIndex(void) const
{
    for (size_t nCnt = 0; nCnt < m_Item.size(); nCnt++)
    {
        if (m_Item[nCnt]->IsMouseOver())
        {
            return (int)nCnt;
        }
    }

    return -1;
}