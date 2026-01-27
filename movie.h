//=============================================================================
//
// ムービー処理 [movie.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _MOVIE_H_// このマクロ定義がされていなかったら
#define _MOVIE_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "scene.h"
#include "blockmanager.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CTime;
class CLight;
class CDummyPlayer;

//*****************************************************************************
// ムービークラス
//*****************************************************************************
class CMovie : public CScene
{
public:
	CMovie();
	~CMovie();

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	virtual void ResetLight(void);
	void Draw(void);
	void OnDeviceReset(void) override;
	void ReleaseThumbnail(void) override;
	void ResetThumbnail(void) override;

	static CTime* GetTime(void) { return m_pTime; }

private:
	static constexpr int	FADE_TIME			= 960;		// 画面遷移までの時間
	static constexpr int	EFFECT_CREATE_NUM	= 3;		// エフェクト生成数
	static constexpr float	HEIGHT_STEP			= 30.0f;	// 高さの増加量
	static constexpr int	BLOSSOM_INTERVAL	= 15;		// 桜の生成インターバル

	CBlockManager*	m_pBlockManager;						// ブロックマネージャーへのポインタ
	CLight*			m_pLight;								// ライトへのポインタ
	static CTime*	m_pTime;								// タイムへのポインタ
	int				m_timer;								// 画面遷移タイマー
	int				m_particleTimer;						// パーティクル生成タイマー
	CDummyPlayer*	m_pDummyPlayer;							// ダミープレイヤー
	int				m_smokeTimer;							// 煙発生時間
	int				m_delayTime;							// 煙発生までの遅延時間
	bool			m_smokeActive;							// 煙発生フラグ
};

#endif

