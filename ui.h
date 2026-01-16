//=============================================================================
//
// UI処理 [ui.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _UI_H_// このマクロ定義がされていなかったら
#define _UI_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "Object2D.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CUIBase;

//*****************************************************************************
// UIマネージャークラス
//*****************************************************************************
class CUIManager
{
public:
    static CUIManager* GetInstance(void);

    HRESULT Init(void);
    void Uninit(void);
    void Update(void);
    void Draw(void);

    // UI 登録
    void AddUI(const std::string& name, CUIBase* ui);

    // 名前で取得
    CUIBase* GetUI(const std::string& name);
    const std::vector<CUIBase*>& GetAllUI(void) { return m_uiList; }

private:
    CUIManager() {}
    ~CUIManager() {}

private:
    std::vector<CUIBase*> m_uiList;
    std::unordered_map<std::string, CUIBase*> m_uiMap;
};

//*****************************************************************************
// UIベースクラス
//*****************************************************************************
class CUIBase : public CObject2D
{
public:
	CUIBase(int nPriority = 6);
	virtual ~CUIBase();

    // フェードの種類
    enum class FadeMode
    {
        None,
        FadeIn,
        FadeOut
    };

    // スライドの種類
    enum class SlideMode
    {
        None,
        SlideIn,
        SlideOut
    };

    // UIレイアウト構造体
    struct UILayout
    {
        float anchorX;   // 0.0～1.0
        float anchorY;   // 0.0～1.0
        float widthRate; // 画面比率
        float heightRate;
    };

    static CUIBase* Create(float x, float y, D3DXCOLOR col, float width, float height);

    virtual HRESULT Init(void);
    virtual void Uninit(void);
    virtual void Update(void);
    virtual void Draw(void);

    void SetPath(const char* path)
    {
        if (path == nullptr)
        {
            path = " ";
        }

        strcpy_s(m_szPath, MAX_PATH, path); 
    }
    void SetAnchor(float x, float y)
    { 
        m_useLayout = true;// レイアウト使用
        m_layout.anchorX = x; 
        m_layout.anchorY = y; 
    }

    void SetSizeRate(float width, float height) { m_layout.widthRate = width; m_layout.heightRate = height; }

    // 表示・非表示(即時)
    void Show(void);
    void Hide(void);
    bool IsVisible(void) const { return m_bVisible; }

    // 表示・非表示(フェード)
    void FadeIn(float duration);
    void FadeOut(float duration);

    // スライドイン・アウト処理
    void SlideIn(const D3DXVECTOR3& from, const D3DXVECTOR3& to, float duration);
    void SlideOut(const D3DXVECTOR3& to, float duration);

    bool IsMouseOver(void);

    // 親子 UI
    void AddChild(CUIBase* child);

protected:
    void ApplyAlpha(void); // alphaをSetColへ反映

private:
    int         m_nIdxTexture;		    // テクスチャインデックス
    char        m_szPath[MAX_PATH];     // ファイルパス
    bool        m_bVisible;             // 表示フラグ
    CUIBase*    m_parent;               // 親UI
    std::vector<CUIBase*> m_children;   // 子UI
    float       m_alpha;                // 現在の透明度
    float       m_fadeSpeed;            // 更新で加算する値
    FadeMode    m_fadeMode;             // フェードモード
    D3DXVECTOR3 m_slideStartPos;        // スライド開始位置
    D3DXVECTOR3 m_slideEndPos;          // スライド終了位置
    float       m_slideT;               // 0.0f ～ 1.0f
    float       m_slideSpeed;           // スライドスピード
    SlideMode   m_slideMode;            // スライドモード
    bool        m_useLayout;            // レイアウトの使用フラグ
    UILayout    m_layout;               // レイアウト構造体変数
    D3DXVECTOR3 m_layoutPos;            // レイアウト用位置
    D3DXVECTOR3 m_slideOffset;          // オフセット位置
};

//*****************************************************************************
// UIテクスチャクラス
//*****************************************************************************
class CUITexture : public CUIBase
{
public:
    CUITexture();
    ~CUITexture();

    static CUITexture* Create(const char* path, float anchorX, float anchorY, D3DXCOLOR col, float widthRate, float heightRate);

    virtual HRESULT Init(void) override;
    virtual void Uninit(void) override;
    virtual void Update(void) override;
    virtual void Draw(void) override;

    void SetUVDirtyUse(bool flag) { m_isUVDirty = flag; }

private:
    bool m_isUVDirty;
};

//*****************************************************************************
// UI文字クラス
//*****************************************************************************
class CUIText : public CUIBase
{
public:
    CUIText(int nPriority = 6);
    virtual ~CUIText();

    static CUIText* Create(const std::string& text, float x, float y, int fontSize, D3DXCOLOR col);

    virtual HRESULT Init(void) override;
    virtual void Uninit(void) override;
    virtual void Update(void) override;
    virtual void Draw(void) override;

    // テキスト設定
    void SetText(const std::string& text) { m_text = text; }
    void SetColor(D3DXCOLOR col) { m_color = col; }
    void SetFontSize(int size);

private:
    std::string     m_text;
    D3DXCOLOR       m_color;
    int             m_fontSize;
    LPD3DXFONT      m_pFont;
    RECT            m_drawRect;
};

#endif

