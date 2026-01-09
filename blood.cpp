//=============================================================================
//
// 血痕処理 [blood.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "blood.h"
#include "texture.h"
#include "manager.h"


//=============================================================================
// コンストラクタ
//=============================================================================
CBlood::CBlood(int nPriority) : CObject3D(nPriority)
{
	// 値のクリア
	m_nIdxTexture = 0;		// テクスチャインデックス
}
//=============================================================================
// デストラクタ
//=============================================================================
CBlood::~CBlood()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CBlood* CBlood::Create(D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXCOLOR col, float width, float height)
{
	CBlood* pBlood = new CBlood;

	// nullptrだったら
	if (pBlood == nullptr)
	{
		return nullptr;
	}

	pBlood->SetPos(pos);
	pBlood->SetRot(D3DXToRadian(rot));
	pBlood->SetCol(col);
	pBlood->SetWidth(width);
	pBlood->SetHeight(height);

	// 初期化失敗時
	if (FAILED(pBlood->Init()))
	{
		return nullptr;
	}

	return pBlood;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CBlood::Init(void)
{
	// テクスチャの取得
	m_nIdxTexture = CManager::GetTexture()->RegisterDynamic("data/TEXTURE/blood.png");

	// 3Dオブジェクトの初期化処理
	CObject3D::Init();

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CBlood::Uninit(void)
{
	// 3Dオブジェクトの終了処理
	CObject3D::Uninit();
}
//=============================================================================
// 更新処理
//=============================================================================
void CBlood::Update(void)
{
	// 3Dオブジェクトの更新処理
	CObject3D::Update();

	D3DXCOLOR col = GetCol();

	col.a -= DEC_ALPHA;// アルファ値を徐々に下げる

	// 色の設定
	SetCol(col);

	if (col.a <= 0.0f)
	{
		// 終了処理
		Uninit();
	}
}
//=============================================================================
// 描画処理
//=============================================================================
void CBlood::Draw(void)
{
	// テクスチャの取得
	CTexture* pTexture = CManager::GetTexture();

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// αテストを有効
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);//デフォルトはfalse
	pDevice->SetRenderState(D3DRS_ALPHAREF, 0);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);//0より大きかったら描画

	// テクスチャの設定
	pDevice->SetTexture(0, pTexture->GetAddress(m_nIdxTexture));

	// 3Dオブジェクトの描画処理
	CObject3D::Draw();

	// αテストを無効に戻す
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);//デフォルトはfalse
}
