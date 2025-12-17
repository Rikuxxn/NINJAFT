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

// --- 前方宣言 ---
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

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	virtual void UpdateLight(void);
	void Draw(void);
	void OnDeviceReset(void) override;
	void ReleaseThumbnail(void) override;
	void ResetThumbnail(void) override;

	static CTime* GetTime(void) { return m_pTime; }

private:
	CPlayer* m_pPlayer;					// プレイヤーへのポインタ
	static CTime* m_pTime;						// タイムへのポインタ
	static CBlock* m_pBlock;					// ブロックへのポインタ
	static CBlockManager* m_pBlockManager;		// ブロックマネージャーへのポインタ


	CLight* m_pLight;
	int m_timer;								// パーティクル生成タイマー

};

#endif

