//=============================================================================
//
// チュートリアル処理 [tutorial.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _TUTORIAL_H_// このマクロ定義がされていなかったら
#define _TUTORIAL_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "scene.h"
#include "blockmanager.h"
#include "block.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CPlayer;
class CLight;
class CTime;

//*****************************************************************************
// チュートリアルクラス
//*****************************************************************************
class CTutorial : public CScene
{
public:
	CTutorial();
	~CTutorial();

	enum StartState
	{
		WaitStart,      // 少し待つ
		Hidden,			// UI非表示
		Idle,			// アイドリング
	};

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void UIUpdate(void);
	virtual void UpdateLight(void);
	void Draw(void);
	void OnDeviceReset(void) override;
	void ReleaseThumbnail(void) override;
	void ResetThumbnail(void) override;

	static CTime* GetTime(void) { return m_pTime; }
	static CBlockManager* GetBlockManager(void) { return m_pBlockManager; }

private:
	static constexpr int TUTORIAL_SEED = 1767579496;// チュートリアルで使うお気に入りのシード値

	CPlayer*				m_pPlayer;			// プレイヤーへのポインタ
	static CTime*			m_pTime;			// タイムへのポインタ
	static CBlock*			m_pBlock;			// ブロックへのポインタ
	static CBlockManager*	m_pBlockManager;	// ブロックマネージャーへのポインタ
	CLight*					m_pLight;			// ライトへのポインタ
	int						m_timer;			// パーティクル生成タイマー
	StartState				m_startState;		// UIの状態
	float					m_stateTimer;		// UI遅延タイマー
	bool					m_canControl;		// 操作可能フラグ
};

#endif

