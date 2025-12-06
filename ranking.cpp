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

// 名前空間stdの使用
using namespace std;

//=============================================================================
// コンストラクタ
//=============================================================================
CRanking::CRanking() : CScene(CScene::MODE_RANKING)
{
	// 値のクリア
	m_pRankingManager = nullptr;
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

	// ランキングタイムの生成
	m_pRankTime = CRankTime::Create(600.0f, 170.0f, 62.0f, 78.0f);

	// ランキングデータをセット
	if (m_pRankingManager)
	{
		std::vector<std::pair<int, int>> rankList;

		for (auto& r : m_pRankingManager->GetList())
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
	CInputMouse* pMouse = CManager::GetInputMouse();
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
		(pMouse->GetTrigger(0) || pJoypad->GetTrigger(pJoypad->JOYKEY_A)))
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