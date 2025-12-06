//=============================================================================
//
// タイム処理 [time.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _TIME_H_// このマクロ定義がされていなかったら
#define _TIME_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"
#include "number.h"


// 前方宣言
class CColon;

//*****************************************************************************
// タイムクラス
//*****************************************************************************
class CTime : public CObject
{
public:
	CTime(int nPriority = 7);
	~CTime();

	static CTime* Create(int minutes, int seconds, float baseX, float baseY, float digitWidth, float digitHeight);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Countup(void);
	void Countdown(void);
	void Draw(void);

	bool IsTimeUp(void) { return m_isTimeUp; }

	float GetProgress(void) const;
	D3DXVECTOR3 GetPos(void) { return D3DXVECTOR3(); }
	int GetMinutes(void) { return m_nMinutes; }
	int GetnSeconds(void) { return m_nSeconds; }

private:
	static const int DIGITS = 4;
	CNumber* m_apNumber[DIGITS];
	int m_nMinutes;							// 分
	int m_nSeconds;							// 秒
	int m_nFrameCount;						// フレームカウント
	float m_digitWidth;						// 数字1桁あたりの幅
	float m_digitHeight;					// 数字1桁あたりの高さ
	D3DXVECTOR3 m_basePos;					// 表示の開始位置
	CColon* m_pColon;						// コロンへのポインタ
	int m_nIdxTexture;						// テクスチャインデックス
	int m_nStartMinutes;
	int m_nStartSeconds;
	bool m_isTimeUp;						// タイムアップフラグ

};

//*****************************************************************************
// コロンクラス
//*****************************************************************************
class CColon : public CObject
{
public:
	CColon(int nPriority = 7);
	~CColon();

	static CColon* Create(D3DXVECTOR3 pos, float fWidth, float fHeight);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	D3DXVECTOR3 GetPos(void);

private:
	LPDIRECT3DVERTEXBUFFER9 m_pVtxBuff;		// 頂点バッファ
	D3DXVECTOR3 m_pos;						// 位置
	float m_fWidth, m_fHeight;				// サイズ（幅・高さ）
	int m_nIdxTexture;						// テクスチャインデックス

};

#endif