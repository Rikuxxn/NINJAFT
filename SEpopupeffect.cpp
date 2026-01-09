//=============================================================================
//
// 効果音ポップアップエフェクト処理 [SEpopupeffect.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "SEpopupeffect.h"


//=============================================================================
// コンストラクタ
//=============================================================================
CSEPopupEffect::CSEPopupEffect()
{
	// 値のクリア
	memset(m_szPath, 0, sizeof(m_szPath));	// ファイルパス
	m_nLife = 0;							// 寿命
}
//=============================================================================
// デストラクタ
//=============================================================================
CSEPopupEffect::~CSEPopupEffect()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CSEPopupEffect* CSEPopupEffect::Create(const char* path, D3DXVECTOR3 pos, D3DXCOLOR col, int nLife)
{
	CSEPopupEffect* pSEPopupEffect = new CSEPopupEffect;

	// nullptrだったら
	if (pSEPopupEffect == nullptr)
	{
		return nullptr;
	}

	pSEPopupEffect->SetPath(path);
	pSEPopupEffect->SetPos(pos);
	pSEPopupEffect->SetCol(col);
	pSEPopupEffect->SetLife(nLife);

	// 初期化失敗時
	if (FAILED(pSEPopupEffect->Init()))
	{
		return nullptr;
	}

	return pSEPopupEffect;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CSEPopupEffect::Init(void)
{
	// ビルボードにファイルパスを設定する
	CObjectBillboard::SetPath(m_szPath);

	// エフェクトの初期化
	CEffect::Init();

	return S_OK;
}
//=============================================================================
// 更新処理
//=============================================================================
void CSEPopupEffect::Update(void)
{
	// パラメータの設定
	SetupParameter();

	// 寿命が尽きたら
	m_nLife--;

	if (m_nLife <= 0)
	{
		// 終了
		Uninit();
		return;
	}

	// エフェクトの更新
	CEffect::Update();
}
//=============================================================================
// パラメータ設定処理
//=============================================================================
void CSEPopupEffect::SetupParameter(void)
{
	EffectDesc desc;

	// テクスチャの指定
	desc.path = m_szPath;

	// 位置
	desc.pos = GetPos();

	// 半径を決めてランダム位置にスポーン
	float radiusMax = 80.0f;

	// 0.0～1.0 の乱数
	float r = (rand() % 10000) / 10000.0f;

	// 平方根を取って均一に分布させる
	float radius = sqrtf(r) * radiusMax;

	float angle = ((rand() % 360) / 180.0f) * D3DX_PI;
	float speed = (rand() % 80) / 60.0f + 0.2f;

	// 位置
	D3DXVECTOR3 offPos = GetPos();
	desc.pos.x = offPos.x + cosf(angle) * radius;
	desc.pos.z = offPos.z + sinf(angle) * radius;
	desc.pos.y = offPos.y;

	desc.move.x = cosf(angle) * speed;
	desc.move.z = sinf(angle) * speed;
	desc.move.y = (rand() % 300) / 100.0f + 0.05f; // 少しだけ上方向

	// 色の設定
	desc.col = GetCol();

	// 半径の設定
	desc.fRadius = 35.0f + (rand() % 30);

	// 寿命の設定
	desc.nLife = GetLife();

	// 重力の設定
	desc.fGravity = -0.001f;

	// 半径の減衰量の設定
	desc.fDecRadius = 0.8f;

	// アルファブレンドの設定フラグ
	desc.bBlend = false;

	// エフェクトの設定
	CEffect::Create(desc);
}
