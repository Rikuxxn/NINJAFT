//=============================================================================
//
// シーン処理 [scene.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "scene.h"
#include "title.h"
#include "game.h"
#include "result.h"
#include "ranking.h"
#include "tutorial.h"
#include "movie.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CScene::CScene(MODE mode):CObject(7)
{
	// 値のクリア
	m_mode = mode;
}
//=============================================================================
// デストラクタ
//=============================================================================
CScene::~CScene()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CScene* CScene::Create(MODE mode)
{
	CScene* pScene = nullptr;

	switch (mode)
	{
	case MODE_TITLE:
		// タイトル画面の生成
		pScene = new CTitle();
		break;

	case MODE_TUTORIAL:
		// チュートリアル画面
		pScene = new CTutorial();
		break;

	case MODE_MOVIE:
		// ムービー画面の生成
		pScene = new CMovie;
		break;

	case MODE_GAME:
		// ゲーム画面の生成
		pScene = new CGame();
		break;

	case MODE_RESULT:
		// リザルト画面の生成
		pScene = new CResult();
		break;

	case MODE_RANKING:
		// ランキング画面の生成
		pScene = new CRanking();
		break;

	default:
		break;
	}

	// 初期化失敗時
	if (FAILED(pScene->Init()))
	{
		return nullptr;
	}

	return pScene;
}

D3DXVECTOR3 CScene::GetPos(void)
{
	return D3DXVECTOR3();
}
