//=============================================================================
//
// ゲーム処理 [game.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _GAME_H_// このマクロ定義がされていなかったら
#define _GAME_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "scene.h"
#include "blockmanager.h"
#include "pausemanager.h"
#include "rankingmanager.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CPlayer;
class CEnemy;
class CTime;
class CGrid;
class CLight;

//*****************************************************************************
// ゲームクラス
//*****************************************************************************
class CGame : public CScene
{
public:
	CGame();
	~CGame();

	enum StartState
	{
		WaitStart,      // 少し待つ
		Hidden,			// UI非表示
		Idle,			// アイドリング
		Failure,		// 失敗時
		WaitEnd,		// 終了までの待機
	};

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	virtual void UpdateLight(void);
	void Draw(void);
	void UIUpdate(void);
	void OnDeviceReset(void) override;
	void ReleaseThumbnail(void) override;
	void ResetThumbnail(void) override;

	static CTime* GetTime(void) { return m_pTime; }
	static CBlockManager* GetBlockManager(void) { return m_pBlockManager; }
	static CPauseManager* GetPauseManager(void) { return m_pPauseManager; }
	static bool GetisPaused(void) { return m_isPaused; };
	static void SetEnablePause(bool bPause);
	static int GetSeed(void) { return m_nSeed; }

private:
	static constexpr int WAIT_TIME			= 190;			// 初回の待機時間
	static constexpr int NUM_SUB_ENEMIES	= 30;			// サブ敵の生成数
	static constexpr int BLOSSOM_INTERVAL	= 15;			// 桜の生成インターバル

	std::unique_ptr<CRankingManager> m_pRankingManager;		// ランキングへのポインタ
	CPlayer*						 m_pPlayer;				// プレイヤーへのポインタ
	CEnemy*							 m_pEnemy;				// 敵へのポインタ
	static CTime*					 m_pTime;				// タイムへのポインタ
	static CBlockManager*			 m_pBlockManager;		// ブロックマネージャーへのポインタ
	static CPauseManager*			 m_pPauseManager;		// ポーズマネージャーへのポインタ
	std::unique_ptr<CGrid>			 m_pGrid;				// グリッドへのポインタ
	static bool						 m_isPaused;			// ポーズ中フラグ
	static int						 m_nSeed;				// マップのシード値
	CLight*							 m_pLight;				// ライトへのポインタ
	int								 m_timer;				// パーティクル生成タイマー
	StartState						 m_startState;			// UIの状態
	float							 m_stateTimer;			// UI遅延タイマー
	bool							 m_canControl;			// 操作可能フラグ
};

#endif
