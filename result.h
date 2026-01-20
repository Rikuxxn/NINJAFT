//=============================================================================
//
// リザルト処理 [result.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _RESULT_H_// このマクロ定義がされていなかったら
#define _RESULT_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "scene.h"
#include "resulttime.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CLight;
class CBlockManager;


//*****************************************************************************
// リザルトクラス
//*****************************************************************************
class CResult : public CScene
{
public:
	CResult();
	~CResult();

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	static void SetClearRank(int idx) { m_clearRankIndex = idx; }
	static void SetSoundCount(int nCount) { m_soundCount = nCount; }
	static void SetInsightCount(int nCount) { m_insightCount = nCount; }
	static void SetTreasureCount(int nCount) { m_treasureCount = nCount; }

	static int GetClearRank(void) { return m_clearRankIndex; }
	static int GetTreasureCount(void) { return m_treasureCount; }
	static int GetSoundCount(void) { return m_soundCount; }
	static int GetInsightCount(void) { return m_insightCount; }


	static void ResetLight(void);
	void OnDeviceReset(void) override;
	void ReleaseThumbnail(void) override;
	void ResetThumbnail(void) override;

private:
	static constexpr int DELAY_TIME = 120;	// 表示遅延時間

	static int			m_clearRankIndex;	// クリア時のランクインデックス
	static int			m_soundCount;		// 音の発生数
	static int			m_insightCount;		// 見つかった回数
	static int			m_treasureCount;	// 宝の数
	CLight*				m_pLight;			// ライトへのポインタ
	CBlockManager*		m_pBlockManager;	// ブロックマネージャーへのポインタ
	int					m_timer;			// 表示タイマー
};

#endif
