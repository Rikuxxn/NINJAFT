//=============================================================================
//
// UI処理 [ui.h]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "ui.h"
#include "manager.h"


//=============================================================================
// UIマネージャーのインスタンス生成
//=============================================================================
CUIManager* CUIManager::GetInstance(void)
{
    static CUIManager inst;
    return &inst;
}
//=============================================================================
// UIマネージャーの初期化処理
//=============================================================================
HRESULT CUIManager::Init(void)
{
    // リストを空にする
    m_uiList.clear();
    m_uiMap.clear();

    return S_OK;
}
//=============================================================================
// UIマネージャーの終了処理
//=============================================================================
void CUIManager::Uninit(void)
{
    for (auto ui : m_uiList)
    {
        if (ui)
        {
            ui->Uninit();
            delete ui;
        }
    }

    // リストを空にする
    m_uiList.clear();
    m_uiMap.clear();
}
//=============================================================================
// UIマネージャーの更新処理
//=============================================================================
void CUIManager::Update(void)
{
    for (auto ui : m_uiList)
    {
        if (ui && ui->IsVisible())
        {
            ui->Update();
        }
    }
}
//=============================================================================
// UIマネージャーの描画処理
//=============================================================================
void CUIManager::Draw(void)
{
    for (auto ui : m_uiList)
    {
        // 表示がtrueのUIのみ描画
        if (ui && ui->IsVisible())
        {
            ui->Draw();
        }
    }
}
//=============================================================================
// UI追加
//=============================================================================
void CUIManager::AddUI(const std::string& name, CUIBase* ui)
{
    if (!ui)
    {
        return;
    }

    m_uiList.push_back(ui);
    m_uiMap[name] = ui;
}
//=============================================================================
// 登録した名前でUIを取得
//=============================================================================
CUIBase* CUIManager::GetUI(const std::string& name)
{
    auto it = m_uiMap.find(name);

    if (it != m_uiMap.end())
    {
        return it->second;
    }

    return nullptr;
}


//=============================================================================
// コンストラクタ
//=============================================================================
CUIBase::CUIBase(int nPriority) : CObject2D(nPriority)
{
	// 値のクリア
    memset(m_szPath, 0, sizeof(m_szPath));
    m_nIdxTexture = 0;
    m_bVisible = true;
    m_parent = nullptr;
}
//=============================================================================
// デストラクタ
//=============================================================================
CUIBase::~CUIBase()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CUIBase* CUIBase::Create(float x, float y, D3DXCOLOR col, float width, float height)
{
    CUIBase* pUi = new CUIBase;

    pUi->SetPos(D3DXVECTOR3(x, y, 0.0f));
    pUi->SetCol(col);
    pUi->SetSize(width, height);

    pUi->Init();

    return pUi;
}
//=============================================================================
// 生成処理(テクスチャあり)
//=============================================================================
CUIBase* CUIBase::Create(const char* path, float x,  float y, D3DXCOLOR col, float width, float height)
{
    CUIBase* pUi = new CUIBase;

    pUi->SetPath(path);
    pUi->SetPos(D3DXVECTOR3(x, y, 0.0f));
    pUi->SetCol(col);
    pUi->SetSize(width, height);

    pUi->Init();

    return pUi;
}
//=============================================================================
// 初期化処理処理
//=============================================================================
HRESULT CUIBase::Init(void)
{
    // テクスチャの取得
    m_nIdxTexture = CManager::GetTexture()->RegisterDynamic(m_szPath);

    // 2Dオブジェクトの初期化処理
    CObject2D::Init();

    return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CUIBase::Uninit(void)
{
    for (auto child : m_children)
    {
        child->Uninit();
        delete child;
    }

    // リストを空にする
    m_children.clear();
}
//=============================================================================
// 更新処理
//=============================================================================
void CUIBase::Update(void)
{
    if (!m_bVisible)
    {
        return;
    }

    // 2Dオブジェクトの更新処理
    CObject2D::Update();

    for (auto child : m_children)
    {
        child->Update();
    }
}
//=============================================================================
// 描画処理
//=============================================================================
void CUIBase::Draw(void)
{
    if (!m_bVisible)
    {
        return;
    }

    // デバイスの取得
    LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

    // テクスチャの設定
    pDevice->SetTexture(0, CManager::GetTexture()->GetAddress(m_nIdxTexture));

    // 2Dオブジェクトの描画処理
    CObject2D::Draw();

    for (auto child : m_children)
    {
        child->Draw();
    }
}
//=============================================================================
// マウスカーソル判定
//=============================================================================
bool CUIBase::IsMouseOver(void)
{
    if (!m_bVisible)
    {
        return false;
    }

    // マウス位置を取得（スクリーン座標）
    POINT cursorPos;
    GetCursorPos(&cursorPos);

    // ウィンドウハンドルを取得
    HWND hwnd = GetActiveWindow();

    // スクリーン座標をクライアント座標に変換
    ScreenToClient(hwnd, &cursorPos);

    // UI の範囲を計算
    float left = GetPos().x - GetWidth();
    float right = GetPos().x + GetWidth();
    float top = GetPos().y - GetHeight();
    float bottom = GetPos().y + GetHeight();

    return (cursorPos.x >= left && cursorPos.x <= right &&
        cursorPos.y >= top && cursorPos.y <= bottom);
}


//=============================================================================
// コンストラクタ
//=============================================================================
CUIText::CUIText(int nPriority) : CUIBase(nPriority)
{
    // 値のクリア
    m_pFont = nullptr;
    m_fontSize = 24;
    m_color = D3DXCOLOR(1, 1, 1, 1);
    m_text = "";
}
//=============================================================================
// デストラクタ
//=============================================================================
CUIText::~CUIText()
{
    // なし
}
//=============================================================================
// 生成
//=============================================================================
CUIText* CUIText::Create(const std::string& text, float x, float y, int fontSize, D3DXCOLOR col)
{
    CUIText* ui = new CUIText;

    ui->SetPos(D3DXVECTOR3(x, y, 0.0f));
    ui->m_text = text;
    ui->m_fontSize = fontSize;
    ui->m_color = col;

    // 初期化
    ui->Init();

    // テキスト矩形サイズに合わせてポリゴンサイズを調整
    RECT rc = { 0,0,0,0 };
    ui->m_pFont->DrawText(nullptr, ui->m_text.c_str(), -1, &rc, DT_CALCRECT, D3DCOLOR_ARGB(0, 0, 0, 0));

    // ポリゴンサイズをテキストサイズに合わせる
    float width = static_cast<float>(rc.right - rc.left);
    float height = static_cast<float>(rc.bottom - rc.top);

    // マウス判定用ポリゴンサイズの設定
    ui->SetSize(width * 0.5f, height * 0.5f);

    // マウス判定用ポリゴンは透明
    ui->SetCol(D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.0f));

    return ui;
}
//=============================================================================
// 初期化
//=============================================================================
HRESULT CUIText::Init(void)
{
    // フォント生成
    HDC hDC = GetDC(NULL);
    ReleaseDC(NULL, hDC);

    D3DXCreateFont(
        CManager::GetRenderer()->GetDevice(),
        m_fontSize,
        0,
        FW_NORMAL,
        1,
        FALSE,
        DEFAULT_CHARSET,
        OUT_DEFAULT_PRECIS,
        DEFAULT_QUALITY,
        DEFAULT_PITCH | FF_DONTCARE,
        "YujiSyuku-Regular",
        &m_pFont
    );

    // UIベースの初期化
    CUIBase::Init();

    return S_OK;
}
//=============================================================================
// 終了
//=============================================================================
void CUIText::Uninit(void)
{
    // フォントの破棄
    if (m_pFont != nullptr)
    {
        m_pFont->Release();

        m_pFont = nullptr;
    }

    // UIベースの初期化
    CUIBase::Uninit();
}
//=============================================================================
// 更新
//=============================================================================
void CUIText::Update(void)
{
    if (!IsVisible())
    {
        return;
    }

    CInputMouse* pMouse = CManager::GetInputMouse();
    CFade* pFade = CManager::GetFade();

    if (IsMouseOver())
    {
        if (pMouse->GetTrigger(0))
        {
            // リザルト画面に移行
            pFade->SetFade(CScene::MODE_RESULT);
        }
    }

    // UIベースの更新
    CUIBase::Update();
}
//=============================================================================
// 描画
//=============================================================================
void CUIText::Draw(void)
{
    if (!IsVisible())
    {
        return;
    }

    if (!m_pFont)
    {
        return;
    }

    // 文字サイズを計算
    RECT rcCalc = { 0, 0, 0, 0 };
    m_pFont->DrawText(nullptr, m_text.c_str(), -1, &rcCalc, DT_CALCRECT, 0);

    float textWidth = static_cast<float>(rcCalc.right - rcCalc.left);
    float textHeight = static_cast<float>(rcCalc.bottom - rcCalc.top);

    // ポリゴン中心 (pos) に合わせて左上座標を計算
    RECT rect;
    rect.left = static_cast<LONG>(GetPos().x - textWidth / 2.0f);
    rect.top = static_cast<LONG>(GetPos().y - textHeight / 2.0f);
    rect.right = rect.left + static_cast<LONG>(textWidth);
    rect.bottom = rect.top + static_cast<LONG>(textHeight);

    // 文字描画
    m_pFont->DrawTextA(
        nullptr,
        m_text.c_str(),
        -1,
        &rect,
        DT_NOCLIP,
        m_color
    );

    // UIベースの描画
    CUIBase::Draw();
}

//=============================================================================
// フォントサイズ変更
//=============================================================================
void CUIText::SetFontSize(int size)
{
    m_fontSize = size;

    // デバッグ表示用フォントの破棄
    if (m_pFont != nullptr)
    {
        m_pFont->Release();

        m_pFont = nullptr;
    }

    // 初期化
    Init();
}