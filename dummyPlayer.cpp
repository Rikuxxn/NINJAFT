//=============================================================================
//
// ダミープレイヤー処理 [dummyPlayer.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "dummyPlayer.h"
#include "manager.h"


//=============================================================================
// コンストラクタ
//=============================================================================
CDummyPlayer::CDummyPlayer(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	memset(m_apModel, 0, sizeof(m_apModel));	// モデル(パーツ)へのポインタ
	m_pos = INIT_VEC3;							// 位置
	m_rot = INIT_VEC3;							// 向き
	m_size = D3DXVECTOR3(1.2f, 1.2f, 1.2f);		// サイズ
	m_mtxWorld = {};							// ワールドマトリックス
	m_nNumModel = 0;							// モデル(パーツ)の総数
	m_pShadowS = nullptr;						// ステンシルシャドウへのポインタ
	m_pMotion = nullptr;						// モーションへのポインタ
}
//=============================================================================
// デストラクタ
//=============================================================================
CDummyPlayer::~CDummyPlayer()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CDummyPlayer* CDummyPlayer::Create(D3DXVECTOR3 pos, D3DXVECTOR3 rot, int motionType)
{
	CDummyPlayer* pDummyPlayer = new CDummyPlayer;

	pDummyPlayer->SetPos(pos);
	pDummyPlayer->SetRot(D3DXToRadian(rot));
	pDummyPlayer->SetMotionType(motionType);

	// 初期化処理
	pDummyPlayer->Init();

	return pDummyPlayer;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CDummyPlayer::Init(void)
{
	CModel* pModels[MAX_PARTS];
	int nNumModels = 0;

	// パーツの読み込み
	m_pMotion = CMotion::Load("data/dummy_motion.txt", pModels, nNumModels, MAX);

	for (int nCnt = 0; nCnt < nNumModels && nCnt < MAX_PARTS; nCnt++)
	{
		m_apModel[nCnt] = pModels[nCnt];

		// オフセット考慮
		m_apModel[nCnt]->SetOffsetPos(m_apModel[nCnt]->GetPos());
		m_apModel[nCnt]->SetOffsetRot(m_apModel[nCnt]->GetRot());
	}

	// パーツ数を代入
	m_nNumModel = nNumModels;

	// 最初の向き
	SetRot(D3DXVECTOR3(0.0f, -D3DX_PI, 0.0f));

	// ステンシルシャドウの生成
	m_pShadowS = CShadowS::Create("data/MODELS/stencilshadow.x", m_pos);
	m_pShadowS->SetStencilRef(1);// 個別のステンシルバッファの参照値を設定

	// モーションを設定
	m_pMotion->SetMotion(m_motionType/*, 10*/);

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CDummyPlayer::Uninit(void)
{
	for (int nCnt = 0; nCnt < MAX_PARTS; nCnt++)
	{
		if (m_apModel[nCnt] != nullptr)
		{
			m_apModel[nCnt]->Uninit();
			delete m_apModel[nCnt];
			m_apModel[nCnt] = nullptr;
		}
	}

	if (m_pMotion != nullptr)
	{
		delete m_pMotion;
		m_pMotion = nullptr;
	}

	// オブジェクトの破棄(自分自身)
	this->Release();
}
//=============================================================================
// 更新処理
//=============================================================================
void CDummyPlayer::Update(void)
{
	if (m_pShadowS != nullptr)
	{
		// ステンシルシャドウの位置設定
		m_pShadowS->SetPosition(m_pos);
	}

	// モーションの更新処理
	m_pMotion->Update(m_apModel, m_nNumModel);
}
//=============================================================================
// 描画処理
//=============================================================================
void CDummyPlayer::Draw(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// 計算用マトリックス
	D3DXMATRIX mtxRot, mtxTrans, mtxSize;

	// ワールドマトリックスの初期化
	D3DXMatrixIdentity(&m_mtxWorld);

	// サイズを反映
	D3DXMatrixScaling(&mtxSize, m_size.x, m_size.y, m_size.z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxSize);

	// 向きを反映
	D3DXMatrixRotationYawPitchRoll(&mtxRot, m_rot.y, m_rot.x, m_rot.z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxRot);

	// 位置を反映
	D3DXMatrixTranslation(&mtxTrans, m_pos.x, m_pos.y, m_pos.z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxTrans);

	// ワールドマトリックスを設定
	pDevice->SetTransform(D3DTS_WORLD, &m_mtxWorld);

	for (int nCntMat = 0; nCntMat < m_nNumModel; nCntMat++)
	{
		// モデル(パーツ)の描画
		if (m_apModel[nCntMat])
		{
			m_apModel[nCntMat]->Draw();
		}
	}
}
