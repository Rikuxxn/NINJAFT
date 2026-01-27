//=============================================================================
//
// マップ生成処理 [generateMap.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "generateMap.h"
#include "random"
#include "block.h"
#include "blockmanager.h"
#include "meshfield.h"
#include "waterfield.h"


//=============================================================================
// マップジェネレーターのインスタンス生成
//=============================================================================
CGenerateMap* CGenerateMap::GetInstance(void)
{
	static CGenerateMap inst;
	return &inst;
}
//=============================================================================
// マップランダム生成処理
//=============================================================================
void CGenerateMap::GenerateRandomMap(int seed)
{
	srand(seed);

	// 原点を中心に配置するためのオフセット計算
	const float offsetX = -(GRID_X * AREA_SIZE) * HALF_RATE + AREA_SIZE * HALF_RATE;
	const float offsetZ = -(GRID_Z * AREA_SIZE) * HALF_RATE + AREA_SIZE * HALF_RATE;

	// メッシュフィールド(地形)の生成
	m_pMeshField = CMeshField::Create(D3DXVECTOR3(0.0f, 0.0f, 0.0f), MAP_SIZE_X, MAP_SIZE_Z, MAP_DIV_X, MAP_DIV_Z);

	// 水フィールドの生成
	m_pWater = CWaterField::Create(D3DXVECTOR3(0.0f, WATER_HEIGHT, 0.0f), MAP_SIZE_X, MAP_SIZE_Z, WATER_DIV_X, WATER_DIV_Z);

	if (m_pWater)
	{
		// 川の流れる方向の設定
		m_pWater->SetFlowDir(m_pMeshField->GetRiverFlowDir());
	}

	// クラスタ生成
	GenerateClusters(GRID_X, GRID_Z, AREA_SIZE, offsetX, offsetZ);

	// 灯籠補充
	std::vector<D3DXVECTOR3> torchPositions;
	EnsureTorchCount(GRID_X, GRID_Z, AREA_SIZE, offsetX, offsetZ, torchPositions);

	// 埋蔵金補充
	EnsureBuriedTreasureCount(GRID_X, GRID_Z, AREA_SIZE, offsetX, offsetZ, torchPositions, m_treasurePositions);

	// 外周を草で囲む
	GenerateOuterGrassBelt(GRID_X, GRID_Z, AREA_SIZE, offsetX, offsetZ);

	// 巡回ポイントの生成
	D3DXVECTOR3 mapCenter(0.0f, 0.0f, 0.0f);
	GeneratePatrolPoints(mapCenter, PATROL_GAP, torchPositions, PATROL_SAFE_DISTANCE, m_patrolPoints);
}
//=============================================================================
// 地形ランダム生成処理
//=============================================================================
void CGenerateMap::GenerateRandomTerrain(int seed)
{
	srand(seed);

	// メッシュフィールド(地形)の生成
	m_pMeshField = CMeshField::Create(D3DXVECTOR3(0.0f, 0.0f, 0.0f), MAP_SIZE_X, MAP_SIZE_Z, MAP_DIV_X, MAP_DIV_Z);

	// 水フィールドの生成
	m_pWater = CWaterField::Create(D3DXVECTOR3(0.0f, WATER_HEIGHT, 0.0f), MAP_SIZE_X, MAP_SIZE_Z, WATER_DIV_X, WATER_DIV_Z);

	if (m_pWater)
	{
		// 川の流れる方向の設定
		m_pWater->SetFlowDir(m_pMeshField->GetRiverFlowDir());
	}
}
//=============================================================================
// クラスタ生成処理
//=============================================================================
void CGenerateMap::GenerateClusters(int gridX, int gridZ, float areaSize,
	float offsetX, float offsetZ)
{
	for (int nCnt = 0; nCnt < CLUSTER_COUNT; nCnt++)
	{
		float centerX = offsetX + (rand() % gridX) * areaSize;
		float centerZ = offsetZ + (rand() % gridZ) * areaSize;

		float radius = CLUSTER_RADIUS_MIN + rand() % CLUSTER_RADIUS_VAR;
		int count = CLUSTER_ELEMENT_MIN + rand() % CLUSTER_ELEMENT_VAR;

		for (int nCnt2 = 0; nCnt2 < count; nCnt2++)
		{
			D3DXVECTOR3 pos(
				centerX + cosf((rand() % 360) * D3DX_PI / 180.0f) * ((rand() / (float)RAND_MAX) * radius),
				0.0f,
				centerZ + sinf((rand() % 360) * D3DX_PI / 180.0f) * ((rand() / (float)RAND_MAX) * radius)
			);

			// クラスタの要素生成
			CreateClusterElement(pos, areaSize, gridX, gridZ, offsetX, offsetZ);
		}
	}
}
//=============================================================================
// クラスタの要素生成処理
//=============================================================================
void CGenerateMap::CreateClusterElement(const D3DXVECTOR3& pos, float areaSize,
	int gridX, int gridZ, float offsetX, float offsetZ)
{
	// マップの中心座標を求める
	const float mapCenterX = offsetX + (gridX - 1) * areaSize * HALF_RATE;
	const float mapCenterZ = offsetZ + (gridZ - 1) * areaSize * HALF_RATE;
	const float halfWidth = (gridX * areaSize) * HALF_RATE;

	// 中心からの距離を計算
	float distX = fabsf(pos.x - mapCenterX);
	float distZ = fabsf(pos.z - mapCenterZ);

	// 外周 → 草
	if (distX > halfWidth * OUTER_GRASS_THRESHOLD_RATE || distZ > halfWidth * OUTER_GRASS_THRESHOLD_RATE)
	{
		CreateGrassCluster(pos, areaSize, gridX, gridZ, offsetX, offsetZ);
		return;
	}

	CBlock::TYPE type = CBlock::TYPE_GRASS;

	// 川の上だったら飛ばす
	if (m_pMeshField && m_pMeshField->IsRiverCell(pos.x, pos.z, areaSize))
	{
		return;
	}

	CBlock* block = CBlockManager::CreateBlock(type, pos);
	if (!block)
	{
		return;
	}

	if (type == CBlock::TYPE_GRASS)
	{
		ApplyRandomGrassTransform(block);
	}
}
//=============================================================================
// 灯籠補充処理
//=============================================================================
void CGenerateMap::EnsureTorchCount(int gridX, int gridZ, float areaSize,
	float offsetX, float offsetZ, std::vector<D3DXVECTOR3>& torchPositions)
{
	const float startX = offsetX + areaSize * HALF_RATE;
	const float startZ = offsetZ + areaSize * HALF_RATE;
	const float endX = offsetX + (gridX - 1) * areaSize;
	const float endZ = offsetZ + (gridZ - 1) * areaSize;

	const float INWARD_OFFSET = areaSize * TORCH_INWARD_RATE; // 壁から離す距離
	const float variation = areaSize * TORCH_VARIATION_RATE;

	auto randVar = [&]()
	{
		return ((rand() / (float)RAND_MAX) - HALF_RATE) * 2.0f * variation;
	};

	// 出口正面3辺からランダムに2つ選ぶ
	std::vector<TorchSide> sides =
	{
		TORCH_BOTTOM,	// 下
		TORCH_LEFT,		// 左
		TORCH_RIGHT		// 右
	};

	std::shuffle(sides.begin(), sides.end(), std::mt19937{ std::random_device{}() });
	sides.resize(MAX_TORCH);

	for (int side : sides)
	{
		D3DXVECTOR3 pos{};

		switch (side)
		{
		case TORCH_BOTTOM: // 下
			pos = { (startX + endX) * TORCH_FRONT_X_RATE + randVar(), 0.0f, startZ + INWARD_OFFSET };
			break;
		case TORCH_LEFT: // 左
			pos = { startX + INWARD_OFFSET, 0.0f, (startZ + endZ) * TORCH_SIDE_Z_RATE + randVar() };
			break;
		case TORCH_RIGHT: // 右
			pos = { endX - INWARD_OFFSET, 0.0f, (startZ + endZ) * TORCH_SIDE_Z_RATE + randVar() };
			break;
		}

		// 川の上なら飛ばす
		if (m_pMeshField && m_pMeshField->IsRiverArea(pos.x, pos.z))
		{
			continue;
		}

		if (CBlock* torch = CBlockManager::CreateBlock(CBlock::TYPE_TORCH_01, pos))
		{
			D3DXVECTOR3 offPos = pos;
			offPos.y += TORCH_OFFSET_HEIGHT;
			torch->SetPos(offPos);
			torchPositions.push_back(pos);
		}
	}

#ifdef _DEBUG
	if ((int)torchPositions.size() < MAX_TORCH)
	{
		MessageBox(nullptr, "灯籠の配置に失敗しました。", "エラー", MB_OK | MB_ICONERROR);
	}
#endif
}
//=============================================================================
// 埋蔵金補充処理
//=============================================================================
void CGenerateMap::EnsureBuriedTreasureCount(int gridX, int gridZ, float areaSize,
	float offsetX, float offsetZ, const std::vector<D3DXVECTOR3>& torchPositions,
	std::vector<D3DXVECTOR3>& treasurePositions)
{
	// 埋蔵金同士の最低距離(クラスター用)
	const float MIN_CLUSTER_DISTANCE = TREASURE_CLUSTER_MIN_DIST_RATE * areaSize;

	// 埋蔵金同士の最低距離(通常)
	const float MIN_NORMAL_DISTANCE = TREASURE_NORMAL_MIN_DIST_RATE * areaSize;

	const int TOTAL_TREASURE = MAX_TREASURE;
	const float CLUSTER_RADIUS = areaSize * TREASURE_CLUSTER_RADIUS_RATE;

	const float SAFE_MARGIN = CLUSTER_RADIUS; // 壁、ブロック余白

	float minX = offsetX + SAFE_MARGIN;
	float maxX = offsetX + (gridX - 1) * areaSize - SAFE_MARGIN;
	float minZ = offsetZ + SAFE_MARGIN;
	float maxZ = offsetZ + (gridZ - 1) * areaSize - SAFE_MARGIN;

	int clusterSize = TREASURE_CLUSTER_MIN + rand() % (TREASURE_CLUSTER_MAX - TREASURE_CLUSTER_MIN + 1);

	// クラスター中心を決める
	D3DXVECTOR3 clusterCenter;
	bool foundCenter = false;

	for (int nCnt = 0; nCnt < MAX_ATTEMPTS && !foundCenter; nCnt++)
	{
		float cx = minX + ((float)rand() / RAND_MAX) * (maxX - minX);
		float cz = minZ + ((float)rand() / RAND_MAX) * (maxZ - minZ);
		D3DXVECTOR3 center(cx, 0.0f, cz);

		// 灯籠と重なっていたら
		if (IsCollidingWithTorch(center, areaSize, torchPositions))
		{
			continue;
		}

		// 川の上なら飛ばす
		if (m_pMeshField && m_pMeshField->IsRiverCell(center.x, center.z, areaSize))
		{
			continue;
		}

		clusterCenter = center;
		foundCenter = true;
	}

	for (int nCnt = 0; nCnt < clusterSize; nCnt++)
	{
		for (int a = 0; a < MAX_ATTEMPTS; a++)
		{
			float angle = (float)(rand() % 360) * D3DX_PI / 180.0f;
			float radius = ((float)rand() / RAND_MAX) * CLUSTER_RADIUS;

			float x = clusterCenter.x + cosf(angle) * radius;
			float z = clusterCenter.z + sinf(angle) * radius;
			D3DXVECTOR3 pos(x, 0.0f, z);

			if (IsCollidingWithTorch(pos, areaSize, torchPositions))
			{
				continue;
			}

			// 川の上なら飛ばす
			if (m_pMeshField && m_pMeshField->IsRiverArea(pos.x, pos.z))
			{
				continue;
			}

			bool tooClose = false;
			for (auto& t : treasurePositions)
			{
				float dx = t.x - pos.x;
				float dz = t.z - pos.z;
				if (dx * dx + dz * dz < MIN_CLUSTER_DISTANCE * MIN_CLUSTER_DISTANCE)
				{
					tooClose = true;
					break;
				}
			}

			if (tooClose)
			{
				continue;
			}

			CBlock* treasure = CBlockManager::CreateBlock(CBlock::TYPE_BURIED_TREASURE, pos);

			if (treasure)
			{
				treasure->SetPos(pos);

				// 向きをランダムにする
				float rotY = (rand() % 360) * D3DX_PI / 180.0f;
				treasure->SetRot(D3DXVECTOR3(0.0f, rotY, 0.0f));

				treasurePositions.push_back(pos);
				break;
			}
		}
	}

	int attempts = 0;

	while ((int)treasurePositions.size() < TOTAL_TREASURE && attempts < MAX_ATTEMPTS)
	{
		attempts++;

		float randX = offsetX + (rand() % gridX) * areaSize;
		float randZ = offsetZ + (rand() % gridZ) * areaSize;
		D3DXVECTOR3 pos(randX, 0.0f, randZ);

		// 灯籠と被ったら飛ばす
		if (IsCollidingWithTorch(pos, areaSize, torchPositions))
		{
			continue;
		}

		// 水と被ったら飛ばす
		if (m_pMeshField && m_pMeshField->IsRiverArea(pos.x, pos.z))
		{
			continue;
		}

		bool tooClose = false;
		for (auto& t : treasurePositions)
		{
			float dx = t.x - pos.x;
			float dz = t.z - pos.z;
			if (dx * dx + dz * dz < (MIN_NORMAL_DISTANCE * MIN_NORMAL_DISTANCE))
			{
				tooClose = true;
				break;
			}
		}

		// 近すぎたら飛ばす
		if (tooClose)
		{
			continue;
		}

		CBlock* treasure = CBlockManager::CreateBlock(CBlock::TYPE_BURIED_TREASURE, pos);

		if (treasure)
		{
			treasure->SetPos(pos);

			// 向きをランダムにする
			float rotY = (rand() % 360) * D3DX_PI / 180.0f;
			treasure->SetRot(D3DXVECTOR3(0.0f, rotY, 0.0f));

			treasurePositions.push_back(pos);
		}
	}

#ifdef _DEBUG
	if ((int)treasurePositions.size() < TOTAL_TREASURE)
	{
		MessageBox(nullptr, "埋蔵金の配置に失敗しました。", "エラー", MB_OK | MB_ICONERROR);
	}
#endif
}
//=============================================================================
// 外周部に草を生成
//=============================================================================
void CGenerateMap::GenerateOuterGrassBelt(int gridX, int gridZ, float areaSize,
	float offsetX, float offsetZ)
{
	const float startX = offsetX + areaSize * INWARD_OFFSET_RATE;// 内側に少しずらす
	const float startZ = offsetZ + areaSize * INWARD_OFFSET_RATE;
	const float endX = offsetX + (gridX - 1) * areaSize;
	const float endZ = offsetZ + (gridZ - 1) * areaSize;

	const int clusterPerCell = MIN_CLUSTER_PER_CELL + rand() % CLUSTER_PER_CELL_RANGE;		// 1マスあたりの群れの数
	const float step = areaSize * HALF_RATE;				// 1マス内で複数生成するためのステップ
	const float variation = areaSize * HALF_RATE;		// ランダムばらつき幅

	auto getVariation = [&]() { return ((rand() / (float)RAND_MAX) - HALF_RATE) * 2.0f * variation; };

	// 生成する辺をランダムに選ぶ（4辺のうち3つ）
	std::vector<int> sides = 
	{ 
		GRASS_BOTTOM,	// 下
		GRASS_TOP,		// 上
		GRASS_LEFT,		// 左
		GRASS_RIGHT,	// 右
		GRASS_MAX
	};

	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(sides.begin(), sides.end(), g);
	sides.resize(GRASS_MAX - 1); // 上位3つだけ残す

	if (std::find(sides.begin(), sides.end(), GRASS_BOTTOM) != sides.end())
	{
		// 下辺
		for (float x = startX; x <= endX; x += areaSize * HALF_RATE)
		{
			for (int nCnt = 0; nCnt < clusterPerCell; ++nCnt)
			{
				D3DXVECTOR3 pos(x + getVariation(), 0.0f, startZ + getVariation());

				// 川の上だったら飛ばす
				if (m_pMeshField && m_pMeshField->IsRiverCell(pos.x, pos.z, areaSize))
				{
					continue;
				}

				if (CBlock* b = CBlockManager::CreateBlock(CBlock::TYPE_GRASS, pos))
				{
					// 草の変形処理
					ApplyRandomGrassTransform(b);
				}
			}
		}
	}

	if (std::find(sides.begin(), sides.end(), GRASS_TOP) != sides.end())
	{
		// 上辺
		for (float x = startX; x <= endX; x += areaSize * HALF_RATE)
		{
			for (int nCnt = 0; nCnt < clusterPerCell; ++nCnt)
			{
				D3DXVECTOR3 pos(x + getVariation(), 0.0f, endZ + getVariation());

				// 川の上だったら飛ばす
				if (m_pMeshField && m_pMeshField->IsRiverCell(pos.x, pos.z, areaSize))
				{
					continue;
				}

				if (CBlock* b = CBlockManager::CreateBlock(CBlock::TYPE_GRASS, pos))
				{
					// 草の変形処理
					ApplyRandomGrassTransform(b);
				}
			}
		}
	}

	if (std::find(sides.begin(), sides.end(), GRASS_LEFT) != sides.end())
	{
		// 左辺
		for (float z = startZ + areaSize * HALF_RATE; z < endZ; z += areaSize * HALF_RATE)
		{
			for (int nCnt = 0; nCnt < clusterPerCell; ++nCnt)
			{
				D3DXVECTOR3 pos(startX + getVariation(), 0.0f, z + getVariation());

				// 川の上だったら飛ばす
				if (m_pMeshField && m_pMeshField->IsRiverCell(pos.x, pos.z, areaSize))
				{
					continue;
				}

				if (CBlock* b = CBlockManager::CreateBlock(CBlock::TYPE_GRASS, pos))
				{
					// 草の変形処理
					ApplyRandomGrassTransform(b);
				}
			}
		}
	}

	if (std::find(sides.begin(), sides.end(), GRASS_RIGHT) != sides.end())
	{
		// 右辺
		for (float z = startZ + areaSize * HALF_RATE; z < endZ; z += areaSize * HALF_RATE)
		{
			for (int nCnt = 0; nCnt < clusterPerCell; ++nCnt)
			{
				D3DXVECTOR3 pos(endX + getVariation(), 0.0f, z + getVariation());

				// 川の上だったら飛ばす
				if (m_pMeshField && m_pMeshField->IsRiverCell(pos.x, pos.z, areaSize))
				{
					continue;
				}

				if (CBlock* b = CBlockManager::CreateBlock(CBlock::TYPE_GRASS, pos))
				{
					// 草の変形処理
					ApplyRandomGrassTransform(b);
				}
			}
		}
	}
}

//=============================================================================
// 外周部に茂みの連続クラスタを生成
//=============================================================================
void CGenerateMap::CreateGrassCluster(const D3DXVECTOR3& centerPos, float areaSize,
	int gridX, int gridZ, float offsetX, float offsetZ)
{
	int grassLength = GRASS_SET_NUM + rand() % GRASS_SET_NUM_VAL;		// 草を連続配置する数

	// マップ中心を取得
	const float mapCenterX = offsetX + (gridX - 1) * areaSize * HALF_RATE;
	const float mapCenterZ = offsetZ + (gridZ - 1) * areaSize * HALF_RATE;
	const float mapHalfX = (gridX * areaSize) * HALF_RATE;
	const float mapHalfZ = (gridZ * areaSize) * HALF_RATE;

	// 中心からの方向に応じて外向き配置
	D3DXVECTOR3 dir = { 0, 0, 0 };
	if (fabsf(centerPos.x - mapCenterX) > fabsf(centerPos.z - mapCenterZ))
	{
		dir.x = (centerPos.x > mapCenterX) ? 1.0f : -1.0f;
	}
	else
	{
		dir.z = (centerPos.z > mapCenterZ) ? 1.0f : -1.0f;
	}

	for (int nCnt = 0; nCnt < grassLength; nCnt++)
	{
		D3DXVECTOR3 grassPos = centerPos + dir * (nCnt * areaSize);

		// マップ範囲チェック
		if (grassPos.x < offsetX - areaSize || grassPos.x > offsetX + (gridX - 1) * areaSize + areaSize ||
			grassPos.z < offsetZ - areaSize || grassPos.z > offsetZ + (gridZ - 1) * areaSize + areaSize)
		{
			continue;
		}

		// 川の上だったら飛ばす
		if (m_pMeshField && m_pMeshField->IsRiverCell(grassPos.x, grassPos.z, areaSize))
		{
			continue;
		}

		// 草ブロック生成
		CBlock* grassBlock = CBlockManager::CreateBlock(CBlock::TYPE_GRASS, grassPos);
		if (!grassBlock)
		{
			continue;
		}

		// ランダムなスケール・回転を設定
		ApplyRandomGrassTransform(grassBlock);
	}
}
//=============================================================================
// 巡回ポイント生成
//=============================================================================
void CGenerateMap::GeneratePatrolPoints(
	const D3DXVECTOR3& origin,   // 中心点
	float gap,                   // ポイント間の距離
	const std::vector<D3DXVECTOR3>& obstaclePositions, // 灯籠や障害物
	float safeDistance,          // 障害物からの最小距離
	std::vector<D3DXVECTOR3>& outPatrolPoints)
{
	// リストのクリア
	outPatrolPoints.clear();

	float offsets[PATROL_POINT_X] = { -gap, 0.0f, gap };

	auto isNearObstacle = [&](const D3DXVECTOR3& pos) -> bool
	{
		for (const auto& obs : obstaclePositions)
		{
			float dx = pos.x - obs.x;
			float dz = pos.z - obs.z;
			float distSq = dx * dx + dz * dz;
			if (distSq < safeDistance * safeDistance)
			{
				return true;
			}
		}
		return false;
	};

	for (int z = 0; z < PATROL_POINT_X; z++)
	{
		for (int x = 0; x < PATROL_POINT_Z; x++)
		{
			D3DXVECTOR3 pos = origin;
			pos.x += offsets[x];
			pos.z += offsets[z];

			outPatrolPoints.push_back(pos);

#ifdef _DEBUG
			CBlockManager::CreateBlock(CBlock::TYPE_ROCK, pos); // デバッグ用
#endif
		}
	}
}
//=============================================================================
// 埋蔵金が取得されたときにその場所を削除する処理
//=============================================================================
void CGenerateMap::OnTreasureCollected(const D3DXVECTOR3& pos)
{
	for (auto it = m_treasurePositions.begin(); it != m_treasurePositions.end(); ++it)
	{
		// 位置が一致しているか判定
		if (it->x == pos.x && it->z == pos.z)
		{
			// 位置リストからこの位置を削除する
			m_treasurePositions.erase(it);
			break;
		}
	}
}
//=============================================================================
// 灯籠との重なり判定処理
//=============================================================================
bool CGenerateMap::IsCollidingWithTorch(const D3DXVECTOR3& pos, float areaSize, const std::vector<D3DXVECTOR3>& torchPositions)
{
	for (auto& wp : torchPositions)
	{
		if (fabs(wp.x - pos.x) < areaSize * HALF_RATE &&
			fabs(wp.z - pos.z) < areaSize * HALF_RATE)
		{
			// 重なった
			return true;
		}
	}

	// 重なっていない
	return false;
}
//=============================================================================
// 草の変形処理
//=============================================================================
void CGenerateMap::ApplyRandomGrassTransform(CBlock* block)
{
	float scaleX = GRASS_SCALE_X_MIN + (rand() / (float)RAND_MAX) * GRASS_SCALE_X_VAR;
	float scaleY = GRASS_SCALE_Y_MIN + (rand() / (float)RAND_MAX) * GRASS_SCALE_Y_VAR;
	float rotY = (rand() % 360) * D3DX_PI / 180.0f;

	block->SetSize(D3DXVECTOR3(scaleX, scaleY, scaleX));
	block->SetRot(D3DXVECTOR3(0.0f, rotY, 0.0f));
}
