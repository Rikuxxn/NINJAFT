//=============================================================================
//
// 水フィールド処理 [ waterfield.cpp ]
// Author: RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "waterfield.h"
#include "manager.h"
#include "texture.h"
#include "meshcylinder.h"
#include "player.h"
#include "specbase.h"
#include "motion.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CWaterField::CWaterField(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	m_pIdx = nullptr;		// インデックスバッファ
	m_pVtx = nullptr;		// 頂点バッファ
	m_pos = INIT_VEC3;		// 位置
	m_rot = INIT_VEC3;		// 向き
	m_mtxWorld = {};		// ワールドマトリックス
	m_WaterFiled = {};		// 構造体変数
}
//=============================================================================
// デストラクタ
//=============================================================================
CWaterField::~CWaterField()
{
	// 無し
}
//=============================================================================
// 生成処理
//=============================================================================
CWaterField* CWaterField::Create(D3DXVECTOR3 pos, float fRadiusX, float fRadiusZ, int nNumX, int nNumZ)
{
	// インスタンス生成
	CWaterField* pWaterField = new CWaterField;

	// nullptrだったら
	if (pWaterField == nullptr)
	{
		return nullptr;
	}

	// オブジェクト設定
	pWaterField->m_pos = pos;
	pWaterField->m_WaterFiled.fRadiusX = fRadiusX;
	pWaterField->m_WaterFiled.fRadiusZ = fRadiusZ;
	pWaterField->m_WaterFiled.nNumX = nNumX;
	pWaterField->m_WaterFiled.nNumZ = nNumZ;
	pWaterField->m_WaterFiled.fSurfaceHeight = pos.y;

	// 初期化失敗時
	if (FAILED(pWaterField->Init()))
	{
		return nullptr;
	}

	return pWaterField;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CWaterField::Init(void)
{
	// デバイスのポインタ
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// テクスチャポインタ取得
	CTexture* pTexture = CManager::GetTexture();

	// テクスチャ割り当て
	m_WaterFiled.nTexIdx = pTexture->RegisterDynamic("data/TEXTURE/.jpg");

	// 頂点計算
	m_WaterFiled.nNumAllVtx = ((m_WaterFiled.nNumX + 1) * (m_WaterFiled.nNumZ + 1)); // 頂点数
	m_WaterFiled.nNumPrimitive = (((m_WaterFiled.nNumX * m_WaterFiled.nNumZ) * 2)) + (4 * (m_WaterFiled.nNumZ - 1)); // ポリゴン数
	m_WaterFiled.nNumIdx = m_WaterFiled.nNumPrimitive + 2; // インデックス数

	// 頂点バッファの生成
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * m_WaterFiled.nNumAllVtx,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_3D,
		D3DPOOL_MANAGED,
		&m_pVtx,
		NULL);

	// インデックスバッファの生成
	pDevice->CreateIndexBuffer(sizeof(WORD) * m_WaterFiled.nNumIdx,
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_MANAGED,
		&m_pIdx,
		NULL);

	// 変数の初期化
	m_rot = INIT_VEC3;

	// 頂点情報のポインタを宣言
	VERTEX_3D* pVtx = nullptr;

	//頂点バッファをロック
	m_pVtx->Lock(0, 0, (void**)&pVtx, 0);

	// テクスチャ座標を計算する変数
	float fTexX = 1.0f / m_WaterFiled.nNumX;
	float fTexY = 1.0f / m_WaterFiled.nNumZ;
	int nCnt = 0;

	D3DXVECTOR3 MathPos = m_pos;

	// 縦
	for (int nCntZ = 0; nCntZ <= m_WaterFiled.nNumZ; nCntZ++)
	{
		// 横
		for (int nCntX = 0; nCntX <= m_WaterFiled.nNumX; nCntX++)
		{
			// 頂点座標を計算
			MathPos.x = ((m_WaterFiled.fRadiusX / m_WaterFiled.nNumX) * nCntX) - (m_WaterFiled.fRadiusX * 0.5f);
			MathPos.y = m_pos.y;
			MathPos.z = m_WaterFiled.fRadiusZ - ((m_WaterFiled.fRadiusZ / m_WaterFiled.nNumZ) * nCntZ) - (m_WaterFiled.fRadiusZ * 0.5f);

			// 頂点座標の設定
			pVtx[nCnt].pos = MathPos;

			// 法線ベクトルの設定
			pVtx[nCnt].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

			// 頂点カラーの設定
			pVtx[nCnt].col = D3DXCOLOR(0.0f, 0.5f, 1.0f, 0.5f);// 半透明の水色;

			// テクスチャ座標の設定
			pVtx[nCnt].tex = D3DXVECTOR2(fTexX * nCntX, nCntZ * fTexY);

			// 加算
			nCnt++;
		}
	}

	// アンロック
	m_pVtx->Unlock();

	// インデックスバッファのポインタ
	WORD* pIdx;

	// インデックスバッファのロック
	m_pIdx->Lock(0, 0, (void**)&pIdx, 0);

	WORD IndxNum = m_WaterFiled.nNumX + 1;// X
	WORD IdxCnt	= 0;// 配列
	WORD Num = 0;

	// インデックスの設定
	for (int IndxCount1 = 0; IndxCount1 < m_WaterFiled.nNumZ; IndxCount1++)
	{
		for (int IndxCount2 = 0; IndxCount2 <= m_WaterFiled.nNumX; IndxCount2++, IndxNum++, Num++)
		{
			pIdx[IdxCnt] = IndxNum;
			pIdx[IdxCnt + 1] = Num;
			IdxCnt += 2;
		}

		// 最後の行じゃなかったら
		if (IndxCount1 < m_WaterFiled.nNumZ - 1)
		{
			pIdx[IdxCnt] = Num - 1;
			pIdx[IdxCnt + 1] = IndxNum;
			IdxCnt += 2;
		}
	}

	// インデックスバッファのアンロック
	m_pIdx->Unlock();

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CWaterField::Uninit(void)
{
	// 頂点バッファの破棄
	if (m_pVtx != nullptr)
	{
		m_pVtx->Release();
		m_pVtx = nullptr;
	}

	// インデックスバッファの破棄
	if (m_pIdx != nullptr)
	{
		m_pIdx->Release();
		m_pIdx = nullptr;
	}

	// 自身の破棄
	this->Release();
}
//=============================================================================
// 更新処理
//=============================================================================
void CWaterField::Update(void)
{
	// 頂点情報のポインタを宣言
	VERTEX_3D* pVtx = nullptr;

	//頂点バッファをロック
	m_pVtx->Lock(0, 0, (void**)&pVtx, 0);

	// テクスチャ座標を計算する変数
	float fTexX = 1.0f / m_WaterFiled.nNumX;
	float fTexY = 1.0f / m_WaterFiled.nNumZ;
	int nCnt = 0;

	D3DXVECTOR3 MathPos = m_pos;

	//縦
	for (int nCntZ = 0; nCntZ <= m_WaterFiled.nNumZ; nCntZ++)
	{
		//横
		for (int nCntX = 0; nCntX <= m_WaterFiled.nNumX; nCntX++)
		{
			// 頂点座標を計算
			MathPos.x = ((m_WaterFiled.fRadiusX / m_WaterFiled.nNumX) * nCntX) - (m_WaterFiled.fRadiusX * 0.5f);
			MathPos.y = m_pos.y;
			MathPos.z = m_WaterFiled.fRadiusZ - ((m_WaterFiled.fRadiusZ / m_WaterFiled.nNumZ) * nCntZ) - (m_WaterFiled.fRadiusZ * 0.5f);

			// 頂点座標の設定
			pVtx[nCnt].pos = MathPos;

			// 法線ベクトルの設定
			pVtx[nCnt].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

			// 頂点カラーの設定
			pVtx[nCnt].col = D3DXCOLOR(0.0f, 0.5f, 1.0f, 0.5f);// 半透明の水色

			// テクスチャ座標の設定
			pVtx[nCnt].tex = D3DXVECTOR2(fTexX * nCntX, nCntZ * fTexY);

			// 加算
			nCnt++;
		}
	}

	// アンロック
	m_pVtx->Unlock();

	// インデックスバッファのポインタ
	WORD* pIdx;

	// インデックスバッファのロック
	m_pIdx->Lock(0, 0, (void**)&pIdx, 0);

	WORD IndxNum = m_WaterFiled.nNumX + 1;// X

	WORD IdxCnt = 0;// 配列

	WORD Num = 0;

	// インデックスの設定
	for (int IndxCount1 = 0; IndxCount1 < m_WaterFiled.nNumZ; IndxCount1++)
	{
		for (int IndxCount2 = 0; IndxCount2 <= m_WaterFiled.nNumX; IndxCount2++, IndxNum++, Num++)
		{
			pIdx[IdxCnt] = IndxNum;
			pIdx[IdxCnt + 1] = Num;
			IdxCnt += 2;
		}

		// 最後の行じゃなかったら
		if (IndxCount1 < m_WaterFiled.nNumZ - 1)
		{
			pIdx[IdxCnt] = Num - 1;
			pIdx[IdxCnt + 1] = IndxNum;
			IdxCnt += 2;
		}
	}

	// インデックスバッファのアンロック
	m_pIdx->Unlock();

	// プレイヤーの位置を取得して、波紋を生成する
	CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();
	D3DXVECTOR3 playerPos = pPlayer->GetPos();

	if (IsInWater(playerPos))
	{
		// 波紋生成
		SpawnCylinder();
	}
}
//=============================================================================
// 描画処理
//=============================================================================
void CWaterField::Draw(void)
{
	// デバイスのポインタ
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// テクスチャの取得
	CTexture* pTexture = CManager::GetTexture();

	// 計算用のマトリックスを宣言
	D3DXMATRIX mtxRot, mtxTrans;

	// ワールドマトリックスの初期化
	D3DXMatrixIdentity(&m_mtxWorld);

	// 向きを反映
	D3DXMatrixRotationYawPitchRoll(&mtxRot, m_rot.y, m_rot.x, m_rot.z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxRot);

	// 位置を反映
	D3DXMatrixTranslation(&mtxTrans, m_pos.x, m_pos.y, m_pos.z);
	D3DXMatrixMultiply(&m_mtxWorld, &m_mtxWorld, &mtxTrans);

	// ワールドマトリックスの設定
	pDevice->SetTransform(D3DTS_WORLD, &m_mtxWorld);

	// 頂点バッファをデバイスのデータストリームに設定
	pDevice->SetStreamSource(0, m_pVtx, 0, sizeof(VERTEX_3D));

	// インデックスバッファをデータストリームに設定
	pDevice->SetIndices(m_pIdx);

	// テクスチャフォーマットの設定
	pDevice->SetFVF(FVF_VERTEX_3D);

	// テクスチャ割り当て
	pDevice->SetTexture(0, pTexture->GetAddress(m_WaterFiled.nTexIdx));

	// ポリゴンの描画
	pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, m_WaterFiled.nNumAllVtx, 0, m_WaterFiled.nNumPrimitive);
}
//=============================================================================
// 波紋生成処理
//=============================================================================
void CWaterField::SpawnCylinder(void)
{
	// 音の取得
	CSound* pSound = CManager::GetSound();

	// プレイヤーの足元に波紋を生成するため、プレイヤーの取得をする
	CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

	if (!pPlayer)
	{
		return;
	}

	// 移動しているときに生成するため、入力を取得する
	InputData input = pPlayer->GatherInput();

	// 波紋生成関数
	auto spawnCylinder = [&]()
	{
		// プレイヤーの位置
		D3DXVECTOR3 pos = pPlayer->GetPos();
		pos.y += 10.0f;

		// 半径を決めてランダム位置にスポーン
		float radiusMax = SPAWN_RADIUS;

		// 0.0～1.0 の乱数
		float r = (rand() % 10000) / 10000.0f;

		// 平方根を取って均一に分布させる
		float radius = sqrtf(r) * radiusMax;

		// 角度
		float angle = ((rand() % 360) / 180.0f) * D3DX_PI;

		// 位置
		pos.x = pos.x + cosf(angle) * radius;
		pos.y += 2.0f;
		pos.z = pos.z + sinf(angle) * radius;

		// 水SEの再生
		if (pSound)
		{
			pSound->StopByLabel(CSound::SOUND_LABEL_WATER);
			pSound->Play(CSound::SOUND_LABEL_WATER);
		}

		// 波紋の生成
		CMeshCylinder::Create(pos, D3DXCOLOR(0.3f, 0.7f, 1.0f, 0.9f), 5.0f, 8.0f, 0.5f, 50, 0.03f);
	};

	IsNotStealthSpec        notStealth;	// ステルス中じゃない
	IsMovingSpec            isMoving;	// 移動中
	IsNotDamageMotionSpec   notDamage;	// ダメージモーションじゃない

	AndSpecification<CPlayer> cond1(notStealth, isMoving);			// 条件の合成(ステルス中じゃない && 移動中)
	AndSpecification<CPlayer> playerConditionSpec(cond1, notDamage);// 条件の合成(cond1 && ダメージモーションじゃない)

	// プレイヤー条件フラグ
	bool playerCondition = playerConditionSpec.IsSatisfiedBy(*pPlayer);

	if (pPlayer && playerCondition)
	{
		// 水SEの再生
		if (pPlayer->GetMotion()->EventMotionRange(CPlayer::MOVE, 1, 9) ||
			pPlayer->GetMotion()->EventMotionRange(CPlayer::INJURY, 1, 20))
		{
			spawnCylinder();
		}
		else if (pPlayer->GetMotion()->EventMotionRange(CPlayer::MOVE, 3, 9) ||
			pPlayer->GetMotion()->EventMotionRange(CPlayer::INJURY, 3, 20))
		{
			spawnCylinder();
		}
	}
}
//=============================================================================
// 水中判定処理
//=============================================================================
bool CWaterField::IsInWater(const D3DXVECTOR3& pos)
{
	// XZ範囲チェック
	float halfX = m_WaterFiled.fRadiusX * 0.5f;
	float halfZ = m_WaterFiled.fRadiusZ * 0.5f;

	if (pos.x < m_pos.x - halfX || pos.x > m_pos.x + halfX)
	{
		return false;
	}

	if (pos.z < m_pos.z - halfZ || pos.z > m_pos.z + halfZ)
	{
		return false;
	}

	// Yチェック（水面より下か）
	if (pos.y > m_WaterFiled.fSurfaceHeight)
	{
		return false;
	}

	return true;
}
