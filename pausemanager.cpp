//=============================================================================
//
// ポーズマネージャー処理 [pausemanager.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "pausemanager.h"
#include "manager.h"
#include "game.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CPauseManager::CPauseManager()
{
    // 値のクリア
    m_pVtxBuff      = nullptr;  // 頂点バッファへのポインタ
    m_SelectedIndex = 0;        // 選択したインデックス
    m_inputLock     = false;    // 入力制限フラグ
}
//=============================================================================
// デストラクタ
//=============================================================================
CPauseManager::~CPauseManager()
{
    // なし
}
//=============================================================================
// 初期化処理
//=============================================================================
void CPauseManager::Init(void)
{
    // デバイスの取得
    LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

    // 頂点バッファの生成
    pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4,
        D3DUSAGE_WRITEONLY,
        FVF_VERTEX_2D,
        D3DPOOL_MANAGED,
        &m_pVtxBuff,
        NULL);

    VERTEX_2D* pVtx;// 頂点情報へのポインタ

    // 頂点バッファをロックし、頂点情報へのポインタを取得
    m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

    // 頂点座標の設定
    pVtx[0].pos = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    pVtx[1].pos = D3DXVECTOR3(1920.0f, 0.0f, 0.0f);
    pVtx[2].pos = D3DXVECTOR3(0.0f, 1080.0f, 0.0f);
    pVtx[3].pos = D3DXVECTOR3(1920.0f, 1080.0f, 0.0f);

    // rhwの設定
    pVtx[0].rhw = 1.0f;
    pVtx[1].rhw = 1.0f;
    pVtx[2].rhw = 1.0f;
    pVtx[3].rhw = 1.0f;

    // 頂点カラーの設定
    pVtx[0].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.5f);
    pVtx[1].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.5f);
    pVtx[2].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.5f);
    pVtx[3].col = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.5f);

    // テクスチャ座標の設定
    pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
    pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
    pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
    pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

    // 頂点バッファをアンロックする
    m_pVtxBuff->Unlock();

    // 空にする
    m_Items.clear();

    // 仮の初期位置（Updateで上書き）
    for (int nCnt = 0; nCnt < ITEM_NUM; nCnt++)
    {
        CPause* pause = nullptr;

        // ポーズ項目の生成
        switch (nCnt)
        {
        case CPause::MENU_CONTINUE:
            pause = CPause::Create(CPause::MENU_CONTINUE, {}, ITEM_W, ITEM_H);
            break;
        case CPause::MENU_RETRY:
            pause = CPause::Create(CPause::MENU_RETRY, {}, ITEM_W, ITEM_H);
            break;
        case CPause::MENU_QUIT:
            pause = CPause::Create(CPause::MENU_QUIT, {}, ITEM_W, ITEM_H);
            break;
        }

        // ポーズの初期化処理
        pause->Init();

        pause->SetSelected(false);
        m_Items.push_back(pause);
    }

    m_SelectedIndex = 0;
    m_Items[m_SelectedIndex]->SetSelected(true);
    m_inputLock = false;
}
//=============================================================================
// 終了処理
//=============================================================================
void CPauseManager::Uninit(void)
{
    // 頂点バッファの破棄
    if (m_pVtxBuff != nullptr)
    {
        m_pVtxBuff->Release();
        m_pVtxBuff = nullptr;
    }

    // 空にする
    m_Items.clear();
}
//=============================================================================
// 更新処理
//=============================================================================
void CPauseManager::Update(void)
{
    if (!CGame::GetisPaused())
    {
        return;
    }

    // バックバッファサイズの取得
    float screenW = (float)CManager::GetRenderer()->GetBackBufferWidth();
    float screenH = (float)CManager::GetRenderer()->GetBackBufferHeight();

    // 横にずらす
    float baseX = screenW * ANCHOR_X;
    float startY = screenH * ANCHOR_Y;

    // サイズも画面サイズに伴って調整
    float itemW = screenW * ITEM_WRATE;
    float itemH = screenH * ITEM_HRATE;
    float spacingY = screenH * SPACING_YRATE;

    // 項目の位置更新
    for (int nCnt = 0; nCnt < (int)m_Items.size(); nCnt++)
    {
        // サイズの更新
        m_Items[nCnt]->SetSize(itemW, itemH);

        D3DXVECTOR3 pos(
            baseX - itemW * 0.5f,
            startY + nCnt * spacingY,
            0.0f
        );

        // 位置の更新
        m_Items[nCnt]->SetPos(pos);
    }

    int mouseOver = GetMouseOverIndex();

    // マウスオーバー時の選択変更＆SE
    if (mouseOver != -1 && mouseOver != m_SelectedIndex)
    {
        m_SelectedIndex = mouseOver;

        // 選択SE
        CManager::GetSound()->Play(CSound::SOUND_LABEL_SELECT);
    }

    // ゲームパッドでの上下移動
    bool up = CManager::GetInputJoypad()->GetTrigger(CInputJoypad::JOYKEY_UP);
    bool down = CManager::GetInputJoypad()->GetTrigger(CInputJoypad::JOYKEY_DOWN);

    if ((up || down) && !m_inputLock)
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
            m_SelectedIndex = (int)m_Items.size() - 1;
        }
        if (m_SelectedIndex >= (int)m_Items.size())
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

            // 各項目の選択時処理
            m_Items[m_SelectedIndex]->Execute();
        }
    }

    // 選択状態更新
    for (size_t nCnt = 0; nCnt < m_Items.size(); nCnt++)
    {
        m_Items[nCnt]->SetSelected(nCnt == static_cast<size_t>(m_SelectedIndex));

        // ポーズの更新処理
        m_Items[nCnt]->Update();
    }
}
//=============================================================================
// 描画処理
//=============================================================================
void CPauseManager::Draw(void)
{
    // デバイスの取得
    LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

    // 頂点バッファをデータストリームに設定
    pDevice->SetStreamSource(0, m_pVtxBuff, 0, sizeof(VERTEX_2D));

    // 頂点フォーマットの設定
    pDevice->SetFVF(FVF_VERTEX_2D);

    // テクスチャの設定
    pDevice->SetTexture(0, nullptr);

    // ポリゴンの描画
    pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

    for (auto item : m_Items)
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
int CPauseManager::GetMouseOverIndex(void) const
{
    for (size_t nCnt = 0; nCnt < m_Items.size(); nCnt++)
    {
        if (m_Items[nCnt]->IsMouseOver())
        {
            return (int)nCnt;
        }
    }

    return -1;
}


