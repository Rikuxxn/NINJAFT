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
#include "imguimaneger.h"
#include "grid.h"
#include "light.h"
#include "blockmanager.h"
#include "pausemanager.h"
#include "objectBillboard.h"
#include "block.h"
#include "rankingmanager.h"
#include "time.h"

// --- 前方宣言 ---
class CPlayer;
class CEnemy;

//*****************************************************************************
// ゲームクラス
//*****************************************************************************
class CGame : public CScene
{
public:
	CGame();
	~CGame();

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	virtual void UpdateLight(void);
	void Draw(void);
	void OnDeviceReset(void) override;
	void ReleaseThumbnail(void) override;
	void ResetThumbnail(void) override;

	static CPlayer* GetPlayer(void) { return m_pPlayer; }
	static CEnemy* GetEnemy(void) { return m_pEnemy; }
	static CTime* GetTime(void) { return m_pTime; }
	static CBlock* GetBlock(void) { return m_pBlock; }
	static CBlockManager* GetBlockManager(void) { return m_pBlockManager; }
	static CObjectBillboard* GetBillboard(void) { return m_pBillboard; }
	static CPauseManager* GetPauseManager(void) { return m_pPauseManager; }
	static bool GetisPaused(void) { return m_isPaused; };
	static void SetEnablePause(bool bPause);
	static int GetSeed(void) { return m_nSeed; }

private:
	static CPlayer* m_pPlayer;					// プレイヤーへのポインタ
	static CEnemy* m_pEnemy;					// 敵へのポインタ
	static CTime* m_pTime;						// タイムへのポインタ
	static CBlock* m_pBlock;					// ブロックへのポインタ
	static CBlockManager* m_pBlockManager;		// ブロックマネージャーへのポインタ
	static CObjectBillboard* m_pBillboard;		// ビルボードへのポインタ
	static CPauseManager* m_pPauseManager;		// ポーズマネージャーへのポインタ
	std::unique_ptr<CGrid> m_pGrid;				// グリッドへのポインタ
	static bool m_isPaused;						// trueならポーズ中
	static int m_nSeed;							// マップのシード値
	std::unique_ptr<CRankingManager> m_pRankingManager;			// ランキングへのポインタ
	CLight* m_pLight;
	int m_timer;								// パーティクル生成タイマー
};

#endif
