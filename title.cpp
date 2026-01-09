//=============================================================================
//
// タイトル処理 [title.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "title.h"
#include "input.h"
#include "manager.h"
#include "dummyPlayer.h"
#include "particle.h"
#include "meshdome.h"

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
CBlockManager* CTitle::m_pBlockManager = nullptr;		// ブロックマネージャーへのポインタ

// 名前空間stdの使用
using namespace std;

//=============================================================================
// コンストラクタ
//=============================================================================
CTitle::CTitle() : CScene(CScene::MODE_TITLE)
{
	// 値のクリア
	m_pVtxBuff			= nullptr;		// 頂点バッファへのポインタ
	m_nIdxTextureTitle	= 0;			// テクスチャインデックス
	m_pLight			= nullptr;		// ライトへのポインタ
	m_timer				= 0;			// タイマー

	for (int nCnt = 0; nCnt < TYPE_MAX; nCnt++)
	{
		m_vertexRanges[nCnt] = { -1, -1 }; // 未使用値で初期化
	}
}
//=============================================================================
// デストラクタ
//=============================================================================
CTitle::~CTitle()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CTitle::Init(void)
{
	// マウスカーソルを表示する
	CManager::GetInputMouse()->SetCursorVisibility(true);

	// ブロックマネージャーの生成
	m_pBlockManager = new CBlockManager;

	// ブロックマネージャーの初期化
	m_pBlockManager->Init();

	// ライトの生成
	m_pLight = new CLight;

	// ライトの初期化
	m_pLight->Init();

	// ライトの再設定処理
	ResetLight();

	// JSONの読み込み
	m_pBlockManager->LoadFromJson("data/title_blockinfo.json");

	// ダミープレイヤーの生成
	CDummyPlayer::Create(D3DXVECTOR3(300.0f, 110.0f, -10.0f), D3DXVECTOR3(0.0f, 180.0f, 0.0f), CDummyPlayer::NEUTRAL);

	// メッシュドームの生成
	CMeshDome::Create(D3DXVECTOR3(0.0f, -50.0f, 0.0f), 2800);

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// テクスチャの取得
	m_nIdxTextureTitle = CManager::GetTexture()->RegisterDynamic("data/TEXTURE/title.png");

	m_vertexRanges[TYPE_FIRST] = { 0, 3 }; // タイトル

	// 頂点バッファの生成
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4 * 2,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&m_pVtxBuff,
		NULL);

	VERTEX_2D* pVtx;// 頂点情報へのポインタ

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	ImageInfo images[1] =
	{
		   { D3DXVECTOR3(420.0f, 270.0f, 0.0f), 320.0f, 130.0f },	// タイトルロゴ
	};

	for (int nCnt = 0; nCnt < 1; nCnt++)
	{
		// 頂点座標の設定
		pVtx[0].pos = D3DXVECTOR3(images[nCnt].pos.x - images[nCnt].width, images[nCnt].pos.y - images[nCnt].height, 0.0f);
		pVtx[1].pos = D3DXVECTOR3(images[nCnt].pos.x + images[nCnt].width, images[nCnt].pos.y - images[nCnt].height, 0.0f);
		pVtx[2].pos = D3DXVECTOR3(images[nCnt].pos.x - images[nCnt].width, images[nCnt].pos.y + images[nCnt].height, 0.0f);
		pVtx[3].pos = D3DXVECTOR3(images[nCnt].pos.x + images[nCnt].width, images[nCnt].pos.y + images[nCnt].height, 0.0f);

		// rhwの設定
		pVtx[0].rhw = 1.0f;
		pVtx[1].rhw = 1.0f;
		pVtx[2].rhw = 1.0f;
		pVtx[3].rhw = 1.0f;

		//頂点カラーの設定
		pVtx[0].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pVtx[1].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pVtx[2].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		pVtx[3].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

		pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
		pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
		pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
		pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

		pVtx += 4;
	}

	// 頂点バッファをアンロックする
	m_pVtxBuff->Unlock();

	// 項目選択の生成
	m_pItemSelect = make_unique<CItemSelect>();

	// 項目選択の初期化処理
	m_pItemSelect->Init();

	// カメラの取得
	CCamera* pCamera = CManager::GetCamera();

	// カメラの初期位置を設定しておく
	pCamera->SetCamParameter(D3DXVECTOR3(437.4f, 116.0f, 90.3f),
		D3DXVECTOR3(124.0f, 265.5f, -630.5f),
		D3DXVECTOR3(-0.19f, 0.41f, 0.0f),
		0.0f);

	// 音の取得
	CSound* pSound = CManager::GetSound();

	// タイトルBGMの再生
	if (pSound)
	{
		pSound->Play(CSound::SOUND_LABEL_TITLEBGM);
	}

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CTitle::Uninit(void)
{
	// ブロックマネージャーの破棄
	if (m_pBlockManager != nullptr)
	{
		m_pBlockManager->Uninit();

		delete m_pBlockManager;
		m_pBlockManager = nullptr;
	}

	// ライトの破棄
	if (m_pLight != nullptr)
	{
		delete m_pLight;
		m_pLight = nullptr;
	}

	// 頂点バッファの破棄
	if (m_pVtxBuff != nullptr)
	{
		m_pVtxBuff->Release();
		m_pVtxBuff = nullptr;
	}
}
//=============================================================================
// 更新処理
//=============================================================================
void CTitle::Update(void)
{
	m_timer++;

	if (m_timer >= 15)// 一定間隔で生成
	{// 塵の生成
		// リセット
		m_timer = 0;

		// パーティクル生成
		CParticle::Create<CBlossomParticle>(INIT_VEC3, CManager::GetCamera()->GetPosR(), D3DXCOLOR(0.8f, 0.8f, 0.8f, 0.4f), 0, 1);
	}

	// 項目選択の更新処理
	m_pItemSelect->Update();

	// ブロックマネージャーの更新処理
	m_pBlockManager->Update();
}
//=============================================================================
// 描画処理
//=============================================================================
void CTitle::Draw(void)
{
	// カメラの取得
	CCamera* pCamera = CManager::GetCamera();

	if (!pCamera->GetMode() == CCamera::MODE_EDIT)
	{
		// カメラの設定
		pCamera->SetCamParameter(D3DXVECTOR3(437.4f, 116.0f, 90.3f),
			D3DXVECTOR3(124.0f, 265.5f, -630.5f),
			D3DXVECTOR3(-0.19f, 0.41f, 0.0f),
			0.0f);
	}

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// 頂点バッファをデータストリームに設定
	pDevice->SetStreamSource(0, m_pVtxBuff, 0, sizeof(VERTEX_2D));

	// 頂点フォーマットの設定
	pDevice->SetFVF(FVF_VERTEX_2D);

	// テクスチャインデックス配列
	int textures[1] = { m_nIdxTextureTitle };

	// 各テクスチャごとに描画
	for (int nCnt = 0; nCnt < TYPE_MAX; nCnt++)
	{
		// テクスチャの設定
		pDevice->SetTexture(0, CManager::GetTexture()->GetAddress(textures[nCnt]));

		// ポリゴンの描画
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, m_vertexRanges[nCnt].start, 2);
	}
}
//=============================================================================
// ライト設定処理
//=============================================================================
void CTitle::ResetLight(void)
{
	// ライトを削除しておく
	CLight::Uninit();

	// 暖色・斜め下方向
	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		D3DXCOLOR(1.0f, 0.65f, 0.4f, 1.0f),    // オレンジ系
		D3DXVECTOR3(-0.3f, -0.8f, 0.2f),       // 西日のように斜め
		D3DXVECTOR3(0.0f, 300.0f, 0.0f)
	);

	// 暖色
	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		D3DXCOLOR(1.0f, 0.65f, 0.4f, 1.0f),    // オレンジ系
		D3DXVECTOR3(0.0f, -0.8f, -0.2f),
		D3DXVECTOR3(0.0f, 300.0f, 0.0f)
	);

	// 薄い青
	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		D3DXCOLOR(0.3f, 0.35f, 0.5f, 1.0f),    // 夕方の空の寒色寄り
		D3DXVECTOR3(0.0f, 1.0f, 0.0f),
		D3DXVECTOR3(0.0f, 0.0f, 0.0f)
	);

	// 環境的な補助光
	CLight::AddLight(
		D3DLIGHT_DIRECTIONAL,
		D3DXCOLOR(0.5f, 0.4f, 0.4f, 1.0f),     // 柔らかめの赤系
		D3DXVECTOR3(0.3f, -0.2f, -0.3f),
		D3DXVECTOR3(0.0f, 0.0f, 0.0f)
	);
}
//=============================================================================
// デバイスリセット通知
//=============================================================================
void CTitle::OnDeviceReset(void)
{
	// ライトの再設定処理
	ResetLight();
}
//=============================================================================
// サムネイルリリース通知
//=============================================================================
void CTitle::ReleaseThumbnail(void)
{
	m_pBlockManager->ReleaseThumbnailRenderTarget();
}
//=============================================================================
// サムネイルリセット通知
//=============================================================================
void CTitle::ResetThumbnail(void)
{
	m_pBlockManager->InitThumbnailRenderTarget(CManager::GetRenderer()->GetDevice());
	m_pBlockManager->GenerateThumbnailsForResources(); // キャッシュも再作成
}
