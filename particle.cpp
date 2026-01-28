//=============================================================================
//
// パーティクル処理 [particle.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "particle.h"


//=============================================================================
// コンストラクタ
//=============================================================================
CParticle::CParticle()
{
	// 値のクリア
	m_Dir			= INIT_VEC3;// 方向
	m_nLife			= 0;		// 寿命
	m_nMaxParticle	= 0;		// 粒子の最大数
}
//=============================================================================
// デストラクタ
//=============================================================================
CParticle::~CParticle()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CParticle::Init(void)
{
	// エフェクトの初期化
	CEffect::Init();

	return S_OK;
}
//=============================================================================
// 更新処理
//=============================================================================
void CParticle::Update(void)
{
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
// 炎パーティクルの初期化処理
//=============================================================================
HRESULT CFireParticle::Init(void)
{
	// テクスチャを設定しておく
	SetPath("data/TEXTURE/fire.jpg");

	// パーティクルの初期化処理
	CParticle::Init();

	return S_OK;
}
//=============================================================================
// 炎パーティクルの更新処理
//=============================================================================
void CFireParticle::Update(void)
{
	int nMaxParticle = GetMaxParticle();

	// パーティクル生成
	for (int nCnt = 0; nCnt < nMaxParticle; nCnt++)//発生させたい粒子の数
	{
		EffectDesc desc;

		// テクスチャの指定
		desc.path = "data/TEXTURE/fire.jpg";

		// 位置
		desc.pos = GetPos();

		// ランダムな角度で横に広がる
		float angle = ((rand() % 360) / 180.0f) * D3DX_PI;
		float speed = (rand() % 120) / 400.0f + 0.2f;

		desc.move.x = cosf(angle) * speed;
		desc.move.z = sinf(angle) * speed;
		desc.move.y = (rand() % 50) / 100.0f + 0.9f; // 上方向

		// 色の設定
		desc.col = GetCol();

		// 半径の設定
		desc.fRadius = 7.0f + (rand() % 7);

		// 寿命の設定
		desc.nLife = GetLife();

		// 重力の設定
		desc.fGravity = 0.0f;

		// 半径の減衰量の設定
		desc.fDecRadius = 0.3f;

		// エフェクトの設定
		CEffect::Create(desc);
	}

	// パーティクルの更新処理
	CParticle::Update();
}


//=============================================================================
// 水しぶきパーティクルの初期化処理
//=============================================================================
HRESULT CWaterParticle::Init(void)
{
	// テクスチャを設定しておく
	SetPath("data/TEXTURE/smoke.jpg");

	// パーティクルの初期化処理
	CParticle::Init();

	return S_OK;
}
//=============================================================================
// 水しぶきパーティクルの更新処理
//=============================================================================
void CWaterParticle::Update(void)
{
	int nMaxParticle = GetMaxParticle();

	// パーティクル生成
	for (int nCnt = 0; nCnt < nMaxParticle; nCnt++)//発生させたい粒子の数
	{
		EffectDesc desc;

		// テクスチャの指定
		desc.path = "data/TEXTURE/smoke.jpg";

		// 位置
		desc.pos = GetPos();

		// ランダムな角度で横に広がる
		float angle = ((rand() % 360) / 180.0f) * D3DX_PI;
		float speed = (rand() % 200) / 80.0f + 0.2f;

		desc.move.x = cosf(angle) * speed;
		desc.move.z = sinf(angle) * speed;
		desc.move.y = (rand() % 300) / 70.0f + 0.05f; // 少しだけ上方向

		// 色の設定
		desc.col = GetCol();

		// 半径の設定
		desc.fRadius = 4.0f + (rand() % 5);

		// 寿命の設定
		desc.nLife = GetLife();

		// 重力の設定
		desc.fGravity = -0.13f;

		// 半径の減衰量の設定
		desc.fDecRadius = 0.1f;

		// エフェクトの設定
		CEffect::Create(desc);
	}

	// パーティクルの更新処理
	CParticle::Update();
}


//=============================================================================
// 埃パーティクルの初期化処理
//=============================================================================
HRESULT CDustParticle::Init(void)
{
	// テクスチャを設定しておく
	SetPath("data/TEXTURE/smoke.jpg");

	// パーティクルの初期化処理
	CParticle::Init();

	return S_OK;
}
//=============================================================================
// 埃パーティクルの更新処理
//=============================================================================
void CDustParticle::Update(void)
{
	int nMaxParticle = GetMaxParticle();

	// パーティクル生成
	for (int nCnt = 0; nCnt < nMaxParticle; nCnt++)//発生させたい粒子の数
	{
		EffectDesc desc;

		// テクスチャの指定
		desc.path = "data/TEXTURE/smoke.jpg";

		// 位置
		desc.pos = GetPos();

		// ランダムな角度で横に広がる
		float angle = ((rand() % 360) / 180.0f) * D3DX_PI;
		float speed = (rand() % 50) / 100.0f + 0.02f;

		desc.move.x = cosf(angle) * speed;
		desc.move.y = (rand() % 20) / 200.0f + 0.02f;
		desc.move.z = sinf(angle) * speed;

		// 色の設定
		desc.col = GetCol();

		// 半径の設定
		desc.fRadius = 10.0f + (rand() % 15);

		// 寿命の設定
		desc.nLife = GetLife();

		// 重力の設定
		desc.fGravity = 0.01f;

		// 半径の減衰量の設定
		desc.fDecRadius = 0.28f;

		// エフェクトの設定
		CEffect::Create(desc);
	}

	// パーティクルの更新処理
	CParticle::Update();
}


//=============================================================================
// 浮遊パーティクルの初期化処理
//=============================================================================
HRESULT CFloatingParticle::Init(void)
{
	// テクスチャを設定しておく
	SetPath("data/TEXTURE/smoke.jpg");

	// パーティクルの初期化処理
	CParticle::Init();

	return S_OK;
}
//=============================================================================
// 浮遊パーティクルの更新処理
//=============================================================================
void CFloatingParticle::Update(void)
{
	int nMaxParticle = GetMaxParticle();

	// パーティクル生成
	for (int nCnt = 0; nCnt < nMaxParticle; nCnt++)//発生させたい粒子の数
	{
		EffectDesc desc;

		// テクスチャの指定
		desc.path = "data/TEXTURE/smoke.jpg";

		// 位置
		desc.pos = GetPos();

		// ランダムな角度で横に広がる
		float angle = ((rand() % 360) / 180.0f) * D3DX_PI;
		float speed = (rand() % 150) / 300.0f + 0.2f;

		desc.move.x = cosf(angle) * speed;
		desc.move.z = sinf(angle) * speed;
		desc.move.y = -((rand() % 300) / 100.0f + 0.9f); // 下方向

		// 色の設定
		desc.col = GetCol();

		// 半径の設定
		desc.fRadius = 5.0f + (rand() % 30);

		// 寿命の設定
		desc.nLife = GetLife();

		// 重力の設定
		desc.fGravity = 0.0f;

		// 半径の減衰量の設定
		desc.fDecRadius = 0.8f;

		// エフェクトの設定
		CEffect::Create(desc);
	}

	// パーティクルの更新処理
	CParticle::Update();
}


//=============================================================================
// 桜パーティクルのコンストラクタ
//=============================================================================
HRESULT CBlossomParticle::Init(void)
{
	// テクスチャを設定しておく
	SetPath("data/TEXTURE/blossom.png");

	// パーティクルの初期化処理
	CParticle::Init();

	return S_OK;
}
//=============================================================================
// 桜パーティクルのコンストラクタ
//=============================================================================
void CBlossomParticle::Update(void)
{
	int nMaxParticle = GetMaxParticle();

	// パーティクル生成
	for (int nCnt = 0; nCnt < nMaxParticle; nCnt++)//発生させたい粒子の数
	{
		EffectDesc desc;

		// テクスチャの指定
		desc.path = "data/TEXTURE/blossom.png";

		// 半径を決めてランダム位置にスポーン
		float radiusMax = 550.0f;

		// 0.0～1.0 の乱数
		float r = (rand() % 10000) / 10000.0f;

		// 平方根を取って均一に分布させる
		float radius = sqrtf(r) * radiusMax;

		float angle = ((rand() % 360) / 180.0f) * D3DX_PI;
		float height = (rand() % 400) - 30.0f; // -30～370くらい

		// 位置
		D3DXVECTOR3 offPos = GetPos();
		desc.pos.x = offPos.x + cosf(angle) * radius;
		desc.pos.z = offPos.z + sinf(angle) * radius;
		desc.pos.y = offPos.y + height;

		// 移動量
		desc.move.x = (rand() % 100 - 50) / 10.0f;
		desc.move.z = (rand() % 100 - 50) / 10.0f;
		desc.move.y = (rand() % 50) / 200.0f - 0.01f;

		// 色の設定
		desc.col = GetCol();

		// 半径の設定
		desc.fRadius = 6.0f + (rand() % 6);

		// 寿命の設定
		desc.nLife = 200 + (rand() % 200);

		// 重力の設定
		desc.fGravity = 0.006f;

		// 半径の減衰量の設定
		desc.fDecRadius = 0.03f;

		// 回転フラグ
		desc.bTurn = true;

		// 回転スピード
		desc.turnSpeed = 0.04f;

		// エフェクトの設定
		CEffect::Create(desc);
	}

	// パーティクルの更新処理
	CParticle::Update();
}


//=============================================================================
// お宝パーティクルの初期化処理
//=============================================================================
HRESULT CTreasureParticle::Init(void)
{
	// テクスチャを設定しておく
	SetPath("data/TEXTURE/treasure_effect.png");

	// パーティクルの初期化処理
	CParticle::Init();

	return S_OK;
}
//=============================================================================
// お宝パーティクルの更新処理
//=============================================================================
void CTreasureParticle::Update(void)
{
	int nMaxParticle = GetMaxParticle();

	// パーティクル生成
	for (int nCnt = 0; nCnt < nMaxParticle; nCnt++)//発生させたい粒子の数
	{
		EffectDesc desc;

		// テクスチャの指定
		desc.path = "data/TEXTURE/treasure_effect.png";

		// 位置
		desc.pos = GetPos();

		// ランダムな角度で横に広がる
		float angle = ((rand() % 360) / 180.0f) * D3DX_PI;
		float speed = (rand() % 200) / 100.0f + 0.2f;

		desc.move.x = cosf(angle) * speed;
		desc.move.z = sinf(angle) * speed;
		desc.move.y = (rand() % 300) / 100.0f + 0.9f; // 上方向

		// 色の設定
		desc.col = GetCol();

		// 半径の設定
		desc.fRadius = 35.0f + (rand() % 40);

		// 寿命の設定
		desc.nLife = GetLife();

		// 重力の設定
		desc.fGravity = 0.0f;

		// 半径の減衰量の設定
		desc.fDecRadius = 1.5f;

		// エフェクトの設定
		CEffect::Create(desc);
	}

	// パーティクルの更新処理
	CParticle::Update();
}


//=============================================================================
// 鬼火パーティクルの初期化処理
//=============================================================================
HRESULT COnibiParticle::Init(void)
{
	// テクスチャを設定しておく
	SetPath("data/TEXTURE/smoke.jpg");

	// パーティクルの初期化処理
	CParticle::Init();

	return S_OK;
}
//=============================================================================
// 鬼火パーティクルの更新処理
//=============================================================================
void COnibiParticle::Update(void)
{
	int nMaxParticle = GetMaxParticle();

	// パーティクル生成
	for (int nCnt = 0; nCnt < nMaxParticle; nCnt++)//発生させたい粒子の数
	{
		EffectDesc desc;

		// テクスチャの指定
		desc.path = "data/TEXTURE/smoke.jpg";

		// 位置
		desc.pos = GetPos();

		// ランダムな角度で横に広がる
		float angle = ((rand() % 360) / 180.0f) * D3DX_PI;
		float speed = (rand() % 120) / 400.0f + 0.2f;

		desc.move.x = cosf(angle) * speed;
		desc.move.z = sinf(angle) * speed;
		desc.move.y = (rand() % 50) / 100.0f + 0.9f; // 上方向

		// 色の設定
		desc.col = GetCol();

		// 半径の設定
		desc.fRadius = 7.0f + (rand() % 7);

		// 寿命の設定
		desc.nLife = GetLife();

		// 重力の設定
		desc.fGravity = 0.0f;

		// 半径の減衰量の設定
		desc.fDecRadius = 0.35f;

		// エフェクトの設定
		CEffect::Create(desc);
	}

	// パーティクルの更新処理
	CParticle::Update();
}


//=============================================================================
// 煙パーティクルの初期化処理
//=============================================================================
HRESULT CSmokeParticle::Init(void)
{
	// テクスチャを設定しておく
	SetPath("data/TEXTURE/smoke_01.png");

	// パーティクルの初期化処理
	CParticle::Init();

	return S_OK;
}
//=============================================================================
// 煙パーティクルの更新処理
//=============================================================================
void CSmokeParticle::Update(void)
{
	int nMaxParticle = GetMaxParticle();

	// パーティクル生成
	for (int nCnt = 0; nCnt < nMaxParticle; nCnt++)// 発生させたい粒子の数
	{
		EffectDesc desc;

		// テクスチャの指定
		desc.path = "data/TEXTURE/smoke_01.png";

		// 位置
		desc.pos = GetPos();

		// ランダムな角度で横に広がる
		float angle = ((rand() % 360) / 180.0f) * D3DX_PI;
		float speed = (rand() % 200) / 250.0f + 0.2f;

		desc.move.x = cosf(angle) * speed;
		desc.move.z = sinf(angle) * speed;
		desc.move.y = (rand() % 100) / 150.0f + 0.9f; // 上方向

		// 色の設定
		desc.col = GetCol();

		// 半径の設定
		desc.fRadius = 21.0f + (rand() % 22);

		// 寿命の設定
		desc.nLife = GetLife();

		// 重力の設定
		desc.fGravity = 0.0f;

		// 半径の減衰量の設定
		desc.fDecRadius = 0.63f;

		// アルファブレンドの設定フラグ
		desc.bBlend = false;
		
		// 回転フラグ
		desc.bTurn = true;

		// 回転スピード
		desc.turnSpeed = 0.04f;

		// エフェクトの設定
		CEffect::Create(desc);
	}

	// パーティクルの更新処理
	CParticle::Update();
}


//=============================================================================
// 血しぶきパーティクルの初期化処理
//=============================================================================
HRESULT CBloodSplatter::Init(void)
{
	// テクスチャを設定しておく
	SetPath("data/TEXTURE/effect000.jpg");

	// パーティクルの初期化処理
	CParticle::Init();

	return S_OK;
}
//=============================================================================
// 血しぶきパーティクルの更新処理
//=============================================================================
void CBloodSplatter::Update(void)
{
	int nMaxParticle = GetMaxParticle();

	// パーティクル生成
	for (int nCnt = 0; nCnt < nMaxParticle; nCnt++)// 発生させたい粒子の数
	{
		EffectDesc desc;

		// テクスチャの指定
		desc.path = "data/TEXTURE/effect000.jpg";

		// 位置
		desc.pos = GetPos();

		// ランダムな角度で横に広がる
		float angle = ((rand() % 360) / -180.0f);
		float speed = (rand() % 300) / 100.0f + 0.2f;

		// 方向
		D3DXVECTOR3 dir = GetDir();

		// 移動量
		desc.move.x = cosf(angle) * speed;
		desc.move.y = (rand() % 20) / 600.0f + 0.9f; // 上方向
		desc.move.z = sinf(angle) * speed;

		// 色の設定
		desc.col = GetCol();

		// 半径の設定
		desc.fRadius = 7.0f + (rand() % 3);

		// 寿命の設定
		desc.nLife = GetLife();

		// 重力の設定
		desc.fGravity = -1.0f;

		// 半径の減衰量の設定
		desc.fDecRadius = 0.3f;

		// エフェクトの設定
		CEffect::Create(desc);
	}

	// パーティクルの更新処理
	CParticle::Update();
}


//=============================================================================
// ダッシュパーティクルの初期化処理
//=============================================================================
HRESULT CDushParticle::Init(void)
{
	// テクスチャを設定しておく
	SetPath("data/TEXTURE/smoke_01.png");

	// パーティクルの初期化処理
	CParticle::Init();

	return S_OK;
}
//=============================================================================
// ダッシュパーティクルの更新処理
//=============================================================================
void CDushParticle::Update(void)
{
	int nMaxParticle = GetMaxParticle();

	// パーティクル生成
	for (int nCnt = 0; nCnt < nMaxParticle; nCnt++)// 発生させたい粒子の数
	{
		EffectDesc desc;

		// テクスチャの指定
		desc.path = "data/TEXTURE/smoke_01.png";

		// 位置
		desc.pos = GetPos();

		// ランダムな角度で横に広がる
		float angle = ((rand() % 360) / 180.0f) * D3DX_PI;
		float speed = (rand() % 200) / 250.0f + 0.2f;

		// 方向
		D3DXVECTOR3 dir = GetDir();

		// 移動量
		desc.move.x = dir.x * speed;
		desc.move.y = (rand() % 50) / 80.0f + 0.9f; // 上方向
		desc.move.z = sinf(angle) * speed;

		// 色の設定
		desc.col = GetCol();

		// 半径の設定
		desc.fRadius = 8.0f + (rand() % 14);

		// 寿命の設定
		desc.nLife = GetLife();

		// 重力の設定
		desc.fGravity = 0.0f;

		// 半径の減衰量の設定
		desc.fDecRadius = 0.53f;

		// アルファブレンドの設定フラグ
		desc.bBlend = false;

		// 回転フラグ
		desc.bTurn = true;

		// 回転スピード
		desc.turnSpeed = 0.1f;

		// エフェクトの設定
		CEffect::Create(desc);
	}

	// パーティクルの更新処理
	CParticle::Update();
}