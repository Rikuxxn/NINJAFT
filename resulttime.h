//=============================================================================
//
// リザルトタイム処理 [resulttime.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _RESULTTIME_H_// このマクロ定義がされていなかったら
#define _RESULTTIME_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"
#include "number.h"
#include "time.h"


//*****************************************************************************
// リザルトタイムクラス
//*****************************************************************************
class CResultTime : public CObject
{
public:
	CResultTime(int nPriority = 7);
	~CResultTime();

	static CResultTime* Create(int minutes, int seconds, float baseX, float baseY, float digitWidth, float digitHeight);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	void SetMinutes(int nMinutes) { m_nMinutes = nMinutes; }
	void SetSeconds(int nSeconds) { m_nSeconds = nSeconds; }
	D3DXVECTOR3 GetPos(void) { return D3DXVECTOR3(); }
	int GetMinutes(void) { return m_nMinutes; }
	int GetnSeconds(void) { return m_nSeconds; }

private:
	static constexpr int DIGITS = 4;// 桁数(分2,秒2)

	CNumber*	m_apNumber[DIGITS];
	int			m_nMinutes;			// 分
	int			m_nSeconds;			// 秒
	int			m_nFrameCount;		// フレームカウント
	float		m_digitWidth;		// 数字1桁あたりの幅
	float		m_digitHeight;		// 数字1桁あたりの高さ
	D3DXVECTOR3 m_basePos;			// 表示の開始位置
	CColon*		m_pColon;			// コロンへのポインタ
	int			m_nIdxTexture;		// テクスチャインデックス

};

#endif