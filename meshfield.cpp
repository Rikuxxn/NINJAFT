//=============================================================================
//
// メッシュフィールド処理 [ meshfield.cpp ]
// Author: RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "meshfield.h"
#include "manager.h"
#include "texture.h"
#include <algorithm>

////*****************************************************************************
//// 定数宣言
////*****************************************************************************
//namespace MESHFIELD
//{
//	constexpr int X_VTX		= 5;											// X方向の分割数
//	constexpr int Z_VTX		= 5;											// Z方向の分割数
//	constexpr int VERTEX	= ((X_VTX + 1) * (Z_VTX + 1));					// 頂点数
//	constexpr int POLYGON	= (((X_VTX * Z_VTX) * 2)) + (4 * (Z_VTX - 1));	// ポリゴン数
//	constexpr int INDEX		= POLYGON + 2;									// インデックス数
//};

//=============================================================================
// コンストラクタ
//=============================================================================
CMeshField::CMeshField(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	m_pIdx = nullptr;		// インデックスバッファ
	m_pVtx = nullptr;		// 頂点バッファ
	m_pos = INIT_VEC3;		// 位置
	m_rot = INIT_VEC3;		// 向き
	m_mtxWorld = {};		// ワールドマトリックス
	m_MeshFiled = {};		// 構造体変数
	m_riverDir = RIVER_X;	// 川の方向
	m_riverCenter = 0.0f;   // X or Z
	m_riverWidth = 0.0f;    // 半径
	m_riverDepth = 0.0f;    // 深さ
}
//=============================================================================
// デストラクタ
//=============================================================================
CMeshField::~CMeshField()
{
	// 無し
}
//=============================================================================
// 生成処理
//=============================================================================
CMeshField* CMeshField::Create(D3DXVECTOR3 pos, float fRadiusX, float fRadiusZ, int nNumX, int nNumZ)
{
	// インスタンス生成
	CMeshField* pMeshField = new CMeshField;

	// nullptrだったら
	if (pMeshField == nullptr)
	{
		return nullptr;
	}

	// オブジェクト設定
	pMeshField->m_pos = pos;
	pMeshField->m_MeshFiled.fRadiusX = fRadiusX;
	pMeshField->m_MeshFiled.fRadiusZ = fRadiusZ;
	pMeshField->m_MeshFiled.nNumX = nNumX;
	pMeshField->m_MeshFiled.nNumZ = nNumZ;

	// 初期化失敗時
	if (FAILED(pMeshField->Init()))
	{
		return nullptr;
	}

	return pMeshField;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CMeshField::Init(void)
{
	// デバイスのポインタ
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// テクスチャポインタ取得
	CTexture* pTexture = CManager::GetTexture();

	// テクスチャ割り当て
	m_MeshFiled.nTexIdx = pTexture->RegisterDynamic("data/TEXTURE/field100.jpg");

	// 頂点計算
	m_MeshFiled.nNumAllVtx = ((m_MeshFiled.nNumX + 1) * (m_MeshFiled.nNumZ + 1)); // 頂点数
	m_MeshFiled.nNumPrimitive = (((m_MeshFiled.nNumX * m_MeshFiled.nNumZ) * 2)) + (4 * (m_MeshFiled.nNumZ - 1)); // ポリゴン数
	m_MeshFiled.nNumIdx = m_MeshFiled.nNumPrimitive + 2; // インデックス数

	// 川の窪み生成パラメータ
	m_riverDir = (rand() % 2 == 0) ? RIVER_X : RIVER_Z;// 川の方向

	float fieldRadius =
		(m_riverDir == RIVER_X)
		? m_MeshFiled.fRadiusZ
		: m_MeshFiled.fRadiusX;

	m_riverCenter = ((rand() / (float)RAND_MAX) - 0.5f) * fieldRadius * 0.6f;// 川の中心
	m_riverWidth = RIVER_WIDTH;// 川の幅
	m_riverDepth = RIVER_DEPTH;// 川の深さ

	// 蛇行ラインの生成
	CreateRiverLine();

	// 頂点バッファの生成
	pDevice->CreateVertexBuffer(sizeof(VERTEX_3D) * m_MeshFiled.nNumAllVtx,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_3D,
		D3DPOOL_MANAGED,
		&m_pVtx,
		NULL);

	// インデックスバッファの生成
	pDevice->CreateIndexBuffer(sizeof(WORD) * m_MeshFiled.nNumIdx,
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
	float fTexX = 1.0f / m_MeshFiled.nNumX;
	float fTexY = 1.0f / m_MeshFiled.nNumZ;
	int nCnt = 0;

	D3DXVECTOR3 MathPos = m_pos;

	// 縦
	for (int nCntZ = 0; nCntZ <= m_MeshFiled.nNumZ; nCntZ++)
	{
		// 横
		for (int nCntX = 0; nCntX <= m_MeshFiled.nNumX; nCntX++)
		{
			// 頂点座標を計算
			MathPos.x = ((m_MeshFiled.fRadiusX / m_MeshFiled.nNumX) * nCntX) - (m_MeshFiled.fRadiusX * 0.5f);
			MathPos.y = m_pos.y;
			MathPos.z = m_MeshFiled.fRadiusZ - ((m_MeshFiled.fRadiusZ / m_MeshFiled.nNumZ) * nCntZ) - (m_MeshFiled.fRadiusZ * 0.5f);

			// 頂点座標の設定
			pVtx[nCnt].pos = MathPos;

			// 法線ベクトルの設定
			pVtx[nCnt].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

			// 頂点カラーの設定
			pVtx[nCnt].col = INIT_XCOL_WHITE;

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

	WORD IndxNum = m_MeshFiled.nNumX + 1;// X
	WORD IdxCnt = 0;// 配列
	WORD Num = 0;

	// インデックスの設定
	for (int IndxCount1 = 0; IndxCount1 < m_MeshFiled.nNumZ; IndxCount1++)
	{
		for (int IndxCount2 = 0; IndxCount2 <= m_MeshFiled.nNumX; IndxCount2++, IndxNum++, Num++)
		{
			pIdx[IdxCnt] = IndxNum;
			pIdx[IdxCnt + 1] = Num;
			IdxCnt += 2;
		}

		// 最後の行じゃなかったら
		if (IndxCount1 < m_MeshFiled.nNumZ - 1)
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
void CMeshField::Uninit(void)
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
void CMeshField::Update(void)
{
	// 頂点情報のポインタを宣言
	VERTEX_3D* pVtx = nullptr;

	//頂点バッファをロック
	m_pVtx->Lock(0, 0, (void**)&pVtx, 0);

	// テクスチャ座標を計算する変数
	float fTexX = 8.0f / m_MeshFiled.nNumX;
	float fTexY = 8.0f / m_MeshFiled.nNumZ;
	int nCnt = 0;

	D3DXVECTOR3 MathPos = m_pos;

	//縦
	for (int nCntZ = 0; nCntZ <= m_MeshFiled.nNumZ; nCntZ++)
	{
		//横
		for (int nCntX = 0; nCntX <= m_MeshFiled.nNumX; nCntX++)
		{
			// 頂点座標を計算
			MathPos.x = ((m_MeshFiled.fRadiusX / m_MeshFiled.nNumX) * nCntX) - (m_MeshFiled.fRadiusX * 0.5f);
			MathPos.y = m_pos.y;
			MathPos.z = m_MeshFiled.fRadiusZ - ((m_MeshFiled.fRadiusZ / m_MeshFiled.nNumZ) * nCntZ) - (m_MeshFiled.fRadiusZ * 0.5f);

			// 空だったら飛ばす
			if (m_riverLine.empty())
			{
				return;
			}

			// --- 川のくぼみ計算 ---
			float dist = 0.0f;

			if (m_riverDir == RIVER_X)
			{
				// X方向に流れる川（各XごとにZ中心がある）
				float centerZ = m_riverLine[nCntX];
				dist = fabsf(MathPos.z - centerZ);
			}
			else // RIVER_Z
			{
				// Z方向に流れる川（各ZごとにX中心がある）
				float centerX = m_riverLine[nCntZ];
				dist = fabsf(MathPos.x - centerX);
			}

			// 深さを取得して窪みを作る
			float depth = GetRiverDepth(dist);
			MathPos.y -= depth;

			// 頂点座標の設定
			pVtx[nCnt].pos = MathPos;

			// 法線ベクトルの設定
			pVtx[nCnt].nor = D3DXVECTOR3(0.0f, 1.0f, 0.0f);

			// 頂点カラーの設定
			pVtx[nCnt].col = D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f);

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

	WORD IndxNum = m_MeshFiled.nNumX + 1;// X

	WORD IdxCnt = 0;// 配列

	WORD Num = 0;

	// インデックスの設定
	for (int IndxCount1 = 0; IndxCount1 < m_MeshFiled.nNumZ; IndxCount1++)
	{
		for (int IndxCount2 = 0; IndxCount2 <= m_MeshFiled.nNumX; IndxCount2++, IndxNum++, Num++)
		{
			pIdx[IdxCnt] = IndxNum;
			pIdx[IdxCnt + 1] = Num;
			IdxCnt += 2;
		}

		// 最後の行じゃなかったら
		if (IndxCount1 < m_MeshFiled.nNumZ - 1)
		{
			pIdx[IdxCnt] = Num - 1;
			pIdx[IdxCnt + 1] = IndxNum;
			IdxCnt += 2;
		}
	}

	// インデックスバッファのアンロック
	m_pIdx->Unlock();
}
//=============================================================================
// 描画処理
//=============================================================================
void CMeshField::Draw(void)
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
	pDevice->SetTexture(0, pTexture->GetAddress(m_MeshFiled.nTexIdx));

	// ポリゴンの描画
	pDevice->DrawIndexedPrimitive(D3DPT_TRIANGLESTRIP, 0, 0, m_MeshFiled.nNumAllVtx, 0, m_MeshFiled.nNumPrimitive);
}
//=============================================================================
// 川の深さの取得
//=============================================================================
float CMeshField::GetRiverDepth(float dist) const
{
	if (dist > m_riverWidth)
	{
		return 0.0f;
	}

	float t = dist / m_riverWidth; // 0～1

	// なめらかなカーブ（放物線）
	return (1.0f - t * t) * m_riverDepth;
}
//=============================================================================
// 蛇行ライン生成処理
//=============================================================================
void CMeshField::CreateRiverLine(void)
{
	if (m_riverDir == RIVER_Z)
	{
		m_riverLine.resize(m_MeshFiled.nNumZ + 1);

		float center = m_riverCenter;
		float maxOffset = m_MeshFiled.fRadiusX * 0.4f;

		float safeSideX = EXIT_HALF_WIDTH + m_riverWidth * 0.5f;
		safeSideX = std::min(safeSideX, maxOffset);

		float sideSign = (rand() % 2 == 0) ? -1.0f : 1.0f;
		float sideX = safeSideX * sideSign;

		for (int z = 0; z <= m_MeshFiled.nNumZ; z++)
		{
			// 出口正面かどうか
			if (z <= m_MeshFiled.nNumZ)
			{
				float t = (float)z / m_MeshFiled.nNumZ; // 0:出口 / 1:少し手前
				t = std::clamp(t, 0.0f, 1.0f);

				float noise =
					((rand() / (float)RAND_MAX) - 0.5f) * 20.0f * t; // 揺れを徐々に減らす

				m_riverLine[z] = sideX + noise;
				continue;
			}

			float delta = ((rand() / (float)RAND_MAX) - 0.5f) * 20.0f;
			center += delta;
			center = std::clamp(center, -maxOffset, maxOffset);

			m_riverLine[z] = center;
		}
	}
	else // RIVER_X
	{
		m_riverLine.resize(m_MeshFiled.nNumX + 1);

		float center = m_riverCenter;
		float maxOffset = m_MeshFiled.fRadiusZ * 0.4f;

		for (int x = 0; x <= m_MeshFiled.nNumX; x++)
		{
			float delta = ((rand() / (float)RAND_MAX) - 0.5f) * 20.0f;
			center += delta;
			center = std::clamp(center, -maxOffset, maxOffset);

			m_riverLine[x] = center;
		}
	}

	// 平滑化(折れ線感を無くす)
	for (int i = 1; i < (int)m_riverLine.size() - 1; i++)
	{
		m_riverLine[i] = 
			(m_riverLine[i - 1] + m_riverLine[i] + m_riverLine[i + 1]) / 3.0f;
	}
}
//=============================================================================
// 川判定関数
//=============================================================================
bool CMeshField::IsRiverArea(float x, float z) const
{
	// 川ラインが空の場合
	if (m_riverLine.empty())
	{
		return false;
	}

	float dist = GetRiverDistance(x, z);
	return GetRiverDepth(dist) > 0.01f;
}
//=============================================================================
// セル単位川判定関数
//=============================================================================
bool CMeshField::IsRiverCell(float cx, float cz, float size)
{
	float h = size * 0.5f;

	return
		IsRiverArea(cx - h, cz - h) ||
		IsRiverArea(cx + h, cz - h) ||
		IsRiverArea(cx - h, cz + h) ||
		IsRiverArea(cx + h, cz + h);
}
//=============================================================================
// 川の距離を計算する関数
//=============================================================================
float CMeshField::GetRiverDistance(float worldX, float worldZ) const
{
	// 空だったらFLT_MAX(無効値)を返す
	if (m_riverLine.empty())
	{
		return FLT_MAX;
	}

	float localX = worldX - m_pos.x;
	float localZ = worldZ - m_pos.z;

	// X方向の川の場合
	if (m_riverDir == RIVER_X)
	{
		// Z座標を0～1の割合に変換
		float zRate = (localZ + m_MeshFiled.fRadiusZ * 0.5f) / m_MeshFiled.fRadiusZ;

		// 川ライン配列のインデックスに変換
		int idx = (int)(zRate * m_MeshFiled.nNumZ);

		// 範囲外チェック
		if (idx < 0 || idx >= (int)m_riverLine.size())
		{
			return FLT_MAX;
		}

		// そのZ位置での川の中心線
		float centerZ = m_riverLine[idx];

		// 川中心線からの距離を返す
		return fabsf(localZ - centerZ);
	}
	// Z方向の川の場合
	else
	{
		// X座標を 0～1 の割合に変換
		float xRate = (localX + m_MeshFiled.fRadiusX * 0.5f) / m_MeshFiled.fRadiusX;

		// 川ライン配列のインデックスに変換
		int idx = (int)(xRate * m_MeshFiled.nNumX);

		// 範囲外チェック
		if (idx < 0 || idx >= (int)m_riverLine.size())
		{
			return FLT_MAX;
		}

		// そのX位置での川の中心線
		float centerX = m_riverLine[idx];

		// 川中心線からの距離
		return fabsf(localX - centerX);
	}
}
//=============================================================================
// 高さを取得する関数
//=============================================================================
float CMeshField::GetHeight(float worldX, float worldZ) const
{
	// ワールド → ローカル
	float localX = worldX - m_pos.x;
	float localZ = worldZ - m_pos.z;

	// メッシュ範囲外(ベースの高さを返す)
	if (localX < -m_MeshFiled.fRadiusX * 0.5f ||
		localX >  m_MeshFiled.fRadiusX * 0.5f ||
		localZ < -m_MeshFiled.fRadiusZ * 0.5f ||
		localZ >  m_MeshFiled.fRadiusZ * 0.5f)
	{
		return m_pos.y;
	}

	// グリッドサイズ
	float cellX = m_MeshFiled.fRadiusX / m_MeshFiled.nNumX;
	float cellZ = m_MeshFiled.fRadiusZ / m_MeshFiled.nNumZ;

	// グリッド座標
	float fx = (localX + m_MeshFiled.fRadiusX * 0.5f) / cellX;
	float fz = (m_MeshFiled.fRadiusZ * 0.5f - localZ) / cellZ;

	// 左上の整数インデックス
	int ix = (int)fx;
	int iz = (int)fz;

	// 範囲クランプ
	ix = std::max(0, std::min(ix, m_MeshFiled.nNumX - 1));
	iz = std::max(0, std::min(iz, m_MeshFiled.nNumZ - 1));

	// 補間係数
	float tx = fx - ix;
	float tz = fz - iz;

	// 4頂点の高さ取得するラムダ関数
	auto getVertexHeight = [&](int x, int z)
	{
		// 頂点のローカル座標
		float vx =
			((m_MeshFiled.fRadiusX / m_MeshFiled.nNumX) * x)
			- (m_MeshFiled.fRadiusX * 0.5f);

		float vz =
			m_MeshFiled.fRadiusZ
			- ((m_MeshFiled.fRadiusZ / m_MeshFiled.nNumZ) * z)
			- (m_MeshFiled.fRadiusZ * 0.5f);

		// 川中心からの距離を計算
		float dist = 0.0f;
		if (m_riverDir == RIVER_X)
		{
			// X方向に流れる川(Zで距離判定)
			float centerZ = m_riverLine[x];
			dist = fabsf(vz - centerZ);
		}
		else
		{
			// Z方向に流れる川（Xで距離判定）
			float centerX = m_riverLine[z];
			dist = fabsf(vx - centerX);
		}

		// ベース高さ - 川の深さ
		return m_pos.y - GetRiverDepth(dist);
	};

	// 4頂点の高さを取得
	float h00 = getVertexHeight(ix, iz);
	float h10 = getVertexHeight(ix + 1, iz);
	float h01 = getVertexHeight(ix, iz + 1);
	float h11 = getVertexHeight(ix + 1, iz + 1);

	// バイリニア補間で滑らかな高さを算出
	float h0 = h00 + (h10 - h00) * tx;
	float h1 = h01 + (h11 - h01) * tx;
	float h = h0 + (h1 - h0) * tz;

	return h;
}
