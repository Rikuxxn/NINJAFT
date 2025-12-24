//=============================================================================
//
// ゲージ処理 [guage.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _GUAGE_H_
#define _GUAGE_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object2D.h"
#include "objectBillboard.h"

// 前方宣言
class CCharacter;
class CBuriedTreasureBlock;

//*****************************************************************************
// ゲージクラス
//*****************************************************************************
class CGuage : public CObject2D
{
public:
	// 種類
	enum GUAGETYPE
	{
		TYPE_NONE = 0,	// 何もしていない状態
		TYPE_GUAGE,		// 緑ゲージ
		TYPE_BACKGUAGE,	// バックゲージ(赤)
		TYPE_FRAME,		// 枠
		TYPE_MAX
	};

	CGuage();
	~CGuage();

	static CGuage* Create(GUAGETYPE type, D3DXVECTOR3 pos, float fWidth, float fHeight);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	void SetTargetCharacter(CCharacter* pChar) { m_pTargetChar = pChar; }

private:
	int m_nIdxTexture;			// テクスチャインデックス
	GUAGETYPE m_type;			// ゲージの種類
	float m_targetRate;			// 実際のHP割合
	float m_currentRate;		// 表示用ゲージ割合（追従用）
	float m_speed;				// 追従速度
	float m_delayTimer;			// 遅延タイマー(バックゲージ用)
	CCharacter* m_pTargetChar;	// このゲージが追従するキャラクター
};

//*****************************************************************************
// 3Dゲージクラス
//*****************************************************************************
class C3DGuage : public CObjectBillboard
{
public:
	// 種類
	enum GUAGETYPE
	{
		TYPE_NONE = 0,	// 何もしていない状態
		TYPE_GUAGE,		// 緑ゲージ
		TYPE_FRAME,		// 枠
		TYPE_MAX
	};

	C3DGuage(int nPriority = 6);
	~C3DGuage();

	static C3DGuage* Create(GUAGETYPE type, D3DXVECTOR3 pos, float fWidth, float fHeight);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

	// 表示・非表示
	void SetActive(bool flag)
	{
		m_bVisible = flag;
		D3DXCOLOR col = GetCol();

		if (m_bVisible)
		{
			col.a = 1.0f;
			SetCol(col);
		}
		else
		{
			col.a = 0.0f;
			SetCol(col);
		}
	}

private:
	GUAGETYPE	m_type;			// ゲージの種類
	bool		m_bVisible;		// 表示フラグ

};

#endif
