//=============================================================================
//
// ランキング処理 [ranking.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "ranking.h"
#include "manager.h"
#include "input.h"
#include "result.h"
#include "background.h"


//=============================================================================
// コンストラクタ
//=============================================================================
CRanking::CRanking() : CScene(CScene::MODE_RANKING)
{
	// 値のクリア
	m_pRankingManager	= nullptr;
	m_pRankItem			= nullptr;
}
//=============================================================================
// デストラクタ
//=============================================================================
CRanking::~CRanking()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CRanking::Init(void)
{
	// ランキングマネージャーのインスタンス生成
	m_pRankingManager = CRankingManager::GetInstance();

	// 背景の生成
	CBackground::Create("data/TEXTURE/ranking_back.png", 0.57f, 0.7f, 0.15f, 0.4f);

	//// タイムランキング
	//TimeRanking(m_pRankingManager);

	// アイテムランキング
	ItemRanking(m_pRankingManager);

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CRanking::Uninit(void)
{
	// ランキングマネージャーの破棄
	if (m_pRankingManager != nullptr)
	{
		delete m_pRankingManager;
		m_pRankingManager = nullptr;
	}
}
//=============================================================================
// 更新処理
//=============================================================================
void CRanking::Update(void)
{
	// 入力受付のためにインプット処理を取得
	CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();
	CInputJoypad* pJoypad = CManager::GetInputJoypad();

	// 画面遷移のためにフェードを取得
	CFade* pFade = CManager::GetFade();

#ifdef _DEBUG
	// 画面遷移
	if (pFade->GetFade() == CFade::FADE_NONE && 
		(pKeyboard->GetAnyKeyTrigger() || pJoypad->GetTrigger(pJoypad->JOYKEY_A)))
	{
		// タイトル画面に移行
		CManager::GetFade()->SetFade(CScene::MODE_TITLE);
	}
#else
	// 画面遷移
	if (pFade->GetFade() == CFade::FADE_NONE && 
		((pKeyboard->GetAnyKeyTrigger()) || pJoypad->GetTrigger(pJoypad->JOYKEY_A)))
	{
		// タイトル画面に移行
		CManager::GetFade()->SetFade(CScene::MODE_TITLE);
	}
#endif
}
//=============================================================================
// 描画処理
//=============================================================================
void CRanking::Draw(void)
{


}
//=============================================================================
// タイムランキング処理
//=============================================================================
void CRanking::TimeRanking(CRankingManager* pRankingManager)
{
	// ランキングタイムの生成
	m_pRankTime = CRankTime::Create(600.0f, 170.0f, 62.0f, 78.0f);

	// ランキングデータをセット
	if (pRankingManager)
	{
		std::vector<std::pair<int, int>> rankList;

		for (auto& r : pRankingManager->GetList())
		{
			rankList.push_back({ r.minutes, r.seconds });
		}

		// タイムリストの設定
		m_pRankTime->SetRankList(rankList);

		int  rankIndex = CResult::GetClearRank();

		if (rankIndex >= 0 && rankIndex < static_cast<int>(rankList.size()))
		{
			// ランクインアニメーション
			m_pRankTime->ShowNewRankEffect(rankIndex);
		}
	}
}
//=============================================================================
// アイテムランキング処理
//=============================================================================
void CRanking::ItemRanking(CRankingManager* pRankingManager)
{
	// ランキングアイテムの生成
	m_pRankItem = CRankItem::Create(0.53f, 0.23f, 0.08f, 0.09f, 1.3f);

	// ランキングデータをセット
	if (pRankingManager)
	{
		std::vector<int> rankList;

		for (auto& r : pRankingManager->GetList())
		{
			rankList.push_back({ r.items });
		}

		// アイテムリストの設定
		m_pRankItem->SetRankList(rankList);

		int  rankIndex = CResult::GetClearRank();

		if (rankIndex >= 0 && rankIndex < static_cast<int>(rankList.size()))
		{
			// ランクインアニメーション
			m_pRankItem->ShowNewRankEffect(rankIndex);
		}
	}
}
