//=============================================================================
//
// ナンバー処理 [number.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _NUMBER_H_// このマクロ定義がされていなかったら
#define _NUMBER_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************

//*****************************************************************************
// ナンバークラス
//*****************************************************************************
class CNumber
{
public:
	CNumber();
	~CNumber();

	static CNumber* Create(float fposX, float fposY, float fWidth, float fHeight);
	HRESULT Init(float fposX, float fposY, float fWidth, float fHeight);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	void SetDigit(int digit);
	void SetPos(D3DXVECTOR3 pos) { m_pos = pos; }
	void SetScaleAnim(void);

private:
	static constexpr float MAX_SCALE = 1.3f;// 最大拡大率

	LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff;		// 頂点バッファへのポインタ
	D3DXVECTOR3				m_pos;			// 位置
	D3DCOLOR				m_col;			// 色
	int						m_digit;		// 桁
	float					m_fWidth;		// 幅
	float					m_fHeight;		// 高さ
	float					m_scale;		// 拡大率
	float					m_easeTime;		// タイマー
	float					m_easeSpeed;	// tの進むスピード
	bool					m_isExpanding;	// true : 拡大, false : 縮小
};

#endif