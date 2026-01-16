//=============================================================================
//
// ゲージ処理 [guage.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "guage.h"
#include "manager.h"
#include "charactermanager.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CGuage::CGuage()
{
	// 値のクリア
	m_nIdxTexture	= 0;			// テクスチャインデックス
	m_type			= TYPE_NONE;	// ゲージの種類
	m_targetRate	= 0.0f;			// 実際のHP割合
	m_currentRate	= 0.0f;			// 表示用ゲージ割合（追従用）
	m_speed			= 0.0f;			// 追従速度
	m_delayTimer	= 0;			// 遅延タイマー(バックゲージ用)
	m_pTargetChar	= nullptr;		// このゲージが追従するキャラクター
}
//=============================================================================
// コンストラクタ
//=============================================================================
CGuage::~CGuage()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CGuage* CGuage::Create(GUAGETYPE type, D3DXVECTOR3 pos, float fWidth, float fHeight)
{
	CGuage* pGuage = new CGuage;

	// nullptrだったら
	if (pGuage == nullptr)
	{
		return nullptr;
	}

	// 初期化処理
	pGuage->SetPos(pos);
	pGuage->SetSize(fWidth, fHeight);
	pGuage->m_type = type;

	if (type == TYPE_GUAGE)
	{// ゲージは緑
		pGuage->SetCol(D3DXCOLOR(0.0f, 1.0f, 0.0f, 1.0f));
	}
	else if (type == TYPE_BACKGUAGE)
	{// バックゲージは赤
		pGuage->SetCol(D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f));
	}
	else if (type == TYPE_FRAME)
	{// フレームはテクスチャの色を使うので白
		pGuage->SetCol(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
	}

	pGuage->m_targetRate = 1.0f;
	pGuage->m_currentRate = 1.0f;
	pGuage->m_speed = 0.005f; // 追従速度

	// 初期化失敗時
	if (FAILED(pGuage->Init()))
	{
		return nullptr;
	}

	return pGuage;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CGuage::Init(void)
{
	if (m_type == TYPE_FRAME)
	{
		// テクスチャ設定
		m_nIdxTexture = CManager::GetTexture()->RegisterDynamic("data/TEXTURE/HpFrame.png");
	}

	// 2Dオブジェクトの初期化処理
	CObject2D::Init();

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CGuage::Uninit(void)
{
	// 2Dオブジェクトの終了処理
	CObject2D::Uninit();
}
//=============================================================================
// 更新処理
//=============================================================================
void CGuage::Update(void)
{
	if (m_type == TYPE_FRAME) 
	{
		UpdateFrame();
		return;
	}

	float fRate = m_pTargetChar->GetHp() / m_pTargetChar->GetMaxHp();
	fRate = std::max(0.0f, std::min(fRate, 1.0f));

	if (m_type == TYPE_GUAGE)
	{
		// 頂点座標を更新
		UpdateGuageVtx(fRate);
	}
	else if (m_type == TYPE_BACKGUAGE)
	{
		// 赤バックゲージは少し遅れて追従
		m_targetRate = fRate;

		if (m_currentRate > m_targetRate)
		{
			m_currentRate -= m_speed;

			if (m_currentRate < m_targetRate)
			{
				m_currentRate = m_targetRate;
			}
		}
		else if (m_currentRate < m_targetRate)
		{
			m_currentRate += m_speed;

			if (m_currentRate > m_targetRate)
			{
				m_currentRate = m_targetRate;
			}
		}

		// 頂点座標を更新
		UpdateGuageVtx(m_currentRate);
	}
}
//=============================================================================
// 描画処理
//=============================================================================
void CGuage::Draw(void)
{
	// テクスチャの取得
	CTexture* pTexture = CManager::GetTexture();

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	if (m_type == TYPE_FRAME)
	{
		// テクスチャの設定
		pDevice->SetTexture(0, pTexture->GetAddress(m_nIdxTexture));
	}
	else
	{// ゲージ自体には適用しない
		// テクスチャの設定
		pDevice->SetTexture(0, nullptr);
	}

	// 2Dオブジェクトの描画処理
	CObject2D::Draw();
}


//=============================================================================
// コンストラクタ
//=============================================================================
C3DGuage::C3DGuage(int nPriority) : CObjectBillboard(nPriority)
{
	// 値のクリア
	m_type		= TYPE_NONE;			// ゲージの種類
	m_bVisible	= false;
}
//=============================================================================
// コンストラクタ
//=============================================================================
C3DGuage::~C3DGuage()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
C3DGuage* C3DGuage::Create(GUAGETYPE type, D3DXVECTOR3 pos, float fWidth, float fHeight)
{
	C3DGuage* pGuage = new C3DGuage;

	// 初期化処理
	pGuage->SetPos(pos);
	pGuage->SetSize(fWidth, fHeight);
	pGuage->m_type = type;

	if (type == TYPE_GUAGE)
	{// ゲージは黄色
		pGuage->SetCol(D3DXCOLOR(1.0f, 1.0f, 0.0f, 1.0f));
	}
	else if (type == TYPE_FRAME)
	{// フレームはテクスチャの色を使うので白
		pGuage->SetCol(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
		pGuage->SetPath("data/TEXTURE/HpFrame.png");
	}

	// 初期化処理
	pGuage->Init();

	return pGuage;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT C3DGuage::Init(void)
{
	// ビルボードオブジェクトの初期化処理
	CObjectBillboard::Init();

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void C3DGuage::Uninit(void)
{
	// ビルボードオブジェクトの終了処理
	CObjectBillboard::Uninit();
}
//=============================================================================
// 更新処理
//=============================================================================
void C3DGuage::Update(void)
{

}
//=============================================================================
// 描画処理
//=============================================================================
void C3DGuage::Draw(void)
{
	// ビルボードオブジェクトの描画処理
	CObjectBillboard::Draw();
}
