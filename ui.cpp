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
#include "result.h"
#include "easing.h"

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
    memset(m_szPath, 0, sizeof(m_szPath));  // ファイルパス
    m_nIdxTexture   = 0;                    // テクスチャインデックス
    m_bVisible      = true;                 // 表示フラグ
    m_parent        = nullptr;              // 親ポインタ
    m_alpha         = 0.0f;                 // アルファ値
    m_fadeSpeed     = 0.0f;                 // フェードスピード
    m_fadeMode      = FadeMode::None;       // フェードモード
    m_slideMode     = SlideMode::None;      // スライドモード
    m_slideT        = 0.0f;                 // スライド割合
    m_slideSpeed    = 0.0f;                 // スライドスピード
    m_useLayout     = false;                // レイアウトの使用フラグ
    m_layoutPos     = INIT_VEC3;            // レイアウト用位置
    m_slideOffset   = INIT_VEC3;            // オフセット位置
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

    // nullptrだったら
    if (pUi == nullptr)
    {
        return nullptr;
    }

    pUi->SetPos(D3DXVECTOR3(x, y, 0.0f));
    pUi->SetCol(col);
    pUi->SetSize(width, height);

    // 初期化失敗時
    if (FAILED(pUi->Init()))
    {
        return nullptr;
    }

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
    // レイアウトを使用する場合
    if (m_useLayout)
    {
        // バックバッファサイズの取得
        float sw = (float)CManager::GetRenderer()->GetBackBufferWidth();
        float sh = (float)CManager::GetRenderer()->GetBackBufferHeight();

        float w = sw * m_layout.widthRate;
        float h = sh * m_layout.heightRate;

        m_layoutPos.x = sw * m_layout.anchorX - w * 0.5f;
        m_layoutPos.y = sh * m_layout.anchorY - h * 0.5f;
        m_layoutPos.z = 0.0f;

        SetSize(w, h);
        //SetPos({ x, y, 0.0f });
    }

    // フェード制御
    if (m_fadeMode == FadeMode::FadeIn)
    {
        m_alpha += m_fadeSpeed;

        if (m_alpha >= 1.0f)
        {
            m_alpha = 1.0f;
            m_fadeMode = FadeMode::None;
        }

        ApplyAlpha();
    }
    else if (m_fadeMode == FadeMode::FadeOut)
    {
        m_alpha -= m_fadeSpeed;

        if (m_alpha <= 0.0f)
        {
            m_alpha = 0.0f;
            m_fadeMode = FadeMode::None;
            m_bVisible = false; // 完全に消えたら非表示状態に
        }

        ApplyAlpha();
    }

    // スライド制御
    if (m_slideMode != SlideMode::None)
    {
        m_slideT += m_slideSpeed;

        if (m_slideT >= 1.0f)
        {
            m_slideT = 1.0f;
            m_slideMode = SlideMode::None;

            if (!m_bVisible)
            {
                // SlideOut完了後に非表示にする場合
                // m_bVisible = false;
            }
        }

        // イージング
        float t = CEasing::Ease(0.0f, 1.0f, m_slideT, CEasing::EaseOutQuint);

        m_slideOffset =
            m_slideStartPos + (m_slideEndPos - m_slideStartPos) * t;
    }

    // 最終座標を合成して更新
    SetPos(m_layoutPos + m_slideOffset);

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
// アルファ値適用処理
//=============================================================================
void CUIBase::ApplyAlpha(void)
{
    SetCol(D3DXCOLOR(1.0f, 1.0f, 1.0f, m_alpha));
}
//=============================================================================
// 表示・非表示処理(即時)
//=============================================================================
void CUIBase::Show(void)
{
    m_bVisible = true;
    m_alpha = 1.0f;
    m_fadeMode = FadeMode::None;
    ApplyAlpha();
}
void CUIBase::Hide(void)
{
    m_bVisible = false;
    m_alpha = 0.0f;
    m_fadeMode = FadeMode::None;
    ApplyAlpha();
}
//=============================================================================
// 表示・非表示処理(フェード)
//=============================================================================
void CUIBase::FadeIn(float duration)
{
    m_bVisible = true;

    // フェード開始時にアルファを0にする
    m_alpha = 0.0f;
    ApplyAlpha();

    m_fadeMode = FadeMode::FadeIn;
    m_fadeSpeed = 1.0f / duration;
}
void CUIBase::FadeOut(float duration)
{
    // フェード開始時にアルファを1にする
    m_alpha = 1.0f;
    ApplyAlpha();

    m_fadeMode = FadeMode::FadeOut;
    m_fadeSpeed = 1.0f / duration;
}
//=============================================================================
// スライドイン・アウト処理
//=============================================================================
void CUIBase::SlideIn(const D3DXVECTOR3& from, const D3DXVECTOR3& to, float duration)
{
    m_bVisible = true;

    m_slideStartPos = from;
    m_slideEndPos   = to;
    m_slideT        = 0.0f;
    m_slideSpeed    = 1.0f / duration;
    m_slideMode     = SlideMode::SlideIn;
}
void CUIBase::SlideOut(const D3DXVECTOR3& to, float duration)
{
    m_slideStartPos = m_slideOffset;
    m_slideEndPos   = to;
    m_slideT        = 0.0f;
    m_slideSpeed    = 1.0f / duration;
    m_slideMode     = SlideMode::SlideOut;
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
// UIテクスチャのコンストラクタ
//=============================================================================
CUITexture::CUITexture()
{
    // 値のクリア
    m_isUVDirty = false;
}
//=============================================================================
// UIテクスチャのデストラクタ
//=============================================================================
CUITexture::~CUITexture()
{
    // なし
}
//=============================================================================
// UIテクスチャ生成処理(位置割合指定)
//=============================================================================
CUITexture* CUITexture::Create(const char* path, float anchorX, float anchorY, D3DXCOLOR col, float widthRate, float heightRate)
{
    CUITexture* pUi = new CUITexture;

    // nullptrだったら
    if (pUi == nullptr)
    {
        return nullptr;
    }

    pUi->SetPath(path);
    pUi->SetAnchor(anchorX, anchorY);
    pUi->SetCol(col);
    pUi->SetSizeRate(widthRate, heightRate);

    // 初期化失敗時
    if (FAILED(pUi->Init()))
    {
        return nullptr;
    }

    return pUi;
}
//=============================================================================
// UIテクスチャ初期化処理
//=============================================================================
HRESULT CUITexture::Init(void)
{
    // UIベースの初期化
    CUIBase::Init();

    return S_OK;
}
//=============================================================================
// UIテクスチャ終了処理
//=============================================================================
void CUITexture::Uninit(void)
{
    // UIベースの終了
    CUIBase::Uninit();
}
//=============================================================================
// UIテクスチャ更新処理
//=============================================================================
void CUITexture::Update(void)
{
    // UIベースの更新
    CUIBase::Update();

    if (!m_isUVDirty)
    {
        return;
    }

    float uvLeft = 0.0f;
    int treasureCount = CResult::GetTreasureCount();
    int soundCount = CResult::GetSoundCount();

    // 次に音発生数に応じてレートを落とす
    float EvaluationRate = 1.0f - soundCount * 0.01f;// 1%ずつ低下
    EvaluationRate = std::max(EvaluationRate, 0.0f); // 最大0%まで下がる

    // 埋蔵金の個数で評価した後に音発生数でレートを下げて、最終決定する
    if (treasureCount >= 8)// Sランク
    {// 埋蔵金8個以上
        uvLeft = 0.0f;   // S

        if (EvaluationRate < 0.9f && EvaluationRate >= 0.8f)
        {// レートが90%より小さい かつ 80%以上
            uvLeft = UV_1;  // A
        }
        else if (EvaluationRate < 0.8f && EvaluationRate >= 0.6f)
        {// レートが80%より小さい かつ 60%以上
            uvLeft = UV_2;   // B
        }
        else if(EvaluationRate <= 0.6f)
        {// レートが50%以下
            uvLeft = UV_3;  // C
        }
    }
    else if (treasureCount >= 5 && treasureCount <= 7)// Aランク
    {// 埋蔵金5個以上7個以下
        uvLeft = UV_1;  // A

        if (EvaluationRate < 0.8f && EvaluationRate >= 0.6f)
        {// レートが80%より小さい かつ 60%以上
            uvLeft = UV_2;   // B
        }
        else if(EvaluationRate <= 0.6f)
        {// レートが60%以下
            uvLeft = UV_3;  // C
        }
    }
    else if (treasureCount >= 3 && treasureCount <= 4)// Bランク
    {// 埋蔵金3個以上4個以下
        uvLeft = UV_2;   // B

        if (EvaluationRate <= 0.8f)
        {// レートが80%以下
            uvLeft = UV_3;  // C
        }
    }
    else
    {// それ以外
        uvLeft = UV_3;  // C
    }

    // テクスチャUV移動処理
    CObject2D::MoveTexUV(uvLeft, 0.0f, UV_1, 1.0f);
}
//=============================================================================
// UIテクスチャ描画処理
//=============================================================================
void CUITexture::Draw(void)
{
    // UIベースの描画
    CUIBase::Draw();
}


//=============================================================================
// UI文字のコンストラクタ
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
// UI文字のデストラクタ
//=============================================================================
CUIText::~CUIText()
{
    // なし
}
//=============================================================================
// UI文字の生成処理
//=============================================================================
CUIText* CUIText::Create(const std::string& text, float x, float y, int fontSize, D3DXCOLOR col)
{
    CUIText* ui = new CUIText;

    // nullptrだったら
    if (ui == nullptr)
    {
        return nullptr;
    }

    ui->SetPos(D3DXVECTOR3(x, y, 0.0f));
    ui->m_text = text;
    ui->m_fontSize = fontSize;
    ui->m_color = col;

    // 初期失敗時
    if (FAILED(ui->Init()))
    {
        return nullptr;
    }

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
// UI文字の初期化処理
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
// UI文字の終了処理
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
// UI文字の更新処理
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
// UI文字の描画処理
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
