//=============================================================================
//
// グリッド処理 [grid.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _GRID_H_
#define _GRID_H_

//*****************************************************************************
// グリッドクラス
//*****************************************************************************
class CGrid
{
public:
    CGrid();
    ~CGrid();

    static std::unique_ptr<CGrid>Create(void);// ユニークポインタの生成
    HRESULT Init(void);
    void Draw(void);
    void AddLine(const D3DXVECTOR3& p1, const D3DXVECTOR3& p2, D3DCOLOR col);

private:
    static constexpr int    SIZE        = 5;    // 半径
    static constexpr float  SPACING     = 80.0f;// マス間隔

    // 頂点構造体
    struct CUSTOMVERTEX
    {
        D3DXVECTOR3 position;
        D3DCOLOR    color;
    };

    std::vector<CUSTOMVERTEX>   m_vertices;   // 頂点
    D3DXMATRIX                  m_mtxWorld;   // ワールドマトリックス
    int                         m_size;       // 半径
    float                       m_spacing;    // マス間隔
};

#endif
