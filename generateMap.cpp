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
#include "game.h"

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
	const float offsetX = -(GRID_X * AREA_SIZE) / 2.0f + AREA_SIZE / 2.0f;
	const float offsetZ = -(GRID_Z * AREA_SIZE) / 2.0f + AREA_SIZE / 2.0f;

	// 小川生成
	std::vector<D3DXVECTOR3> waterPositions;
	GenerateRiver(GRID_X, GRID_Z, AREA_SIZE, offsetX, offsetZ, waterPositions);

	// クラスタ生成
	GenerateClusters(GRID_X, GRID_Z, AREA_SIZE, offsetX, offsetZ, waterPositions);

	// 灯籠補充
	std::vector<D3DXVECTOR3> torchPositions;
	EnsureTorchCount(GRID_X, GRID_Z, AREA_SIZE, offsetX, offsetZ, waterPositions, torchPositions);

	// 埋蔵金補充
	EnsureBuriedTreasureCount(GRID_X, GRID_Z, AREA_SIZE, offsetX, offsetZ, torchPositions, waterPositions, m_treasurePositions);

	// 外周を草で囲む
	GenerateOuterGrassBelt(GRID_X, GRID_Z, AREA_SIZE, offsetX, offsetZ, waterPositions);

	// 床を敷き詰める処理
	FillFloor(GRID_X, GRID_Z, AREA_SIZE);

	// 巡回ポイントの生成
	D3DXVECTOR3 mapCenter(0.0f, 0.0f, 0.0f);
	const float gap = 210.0f; // ポイント間の間隔
	GeneratePatrolPoints(mapCenter, gap, torchPositions, 120.0f, m_patrolPoints);
}
//=============================================================================
// 川生成処理
//=============================================================================
void CGenerateMap::GenerateRiver(int gridX, int gridZ, float areaSize,
	float offsetX, float offsetZ, std::vector<D3DXVECTOR3>& outWaterPositions)
{
	// 川の横幅
	const int riverWidth = 2;

	// 空白数
	int margin = 0;

	bool verticalRiver = (rand() % 2 == 0);
	int x = margin + rand() % (gridX - 2 * margin);
	int z = margin + rand() % (gridZ - 2 * margin);

	if (!verticalRiver)
	{
		// X方向
		for (int i = margin; i < gridX - margin; i++)
		{
			for (int w = 0; w < riverWidth; w++)
			{
				D3DXVECTOR3 pos(offsetX + i * areaSize, -48.0f, offsetZ + (z + w) * areaSize);

				// 川の生成
				CBlockManager::CreateBlock(CBlock::TYPE_WATER, pos);

				outWaterPositions.push_back(pos);
			}

			z += (rand() % 3) - 1;
			z = std::clamp(z, margin, gridZ - margin - riverWidth - 1);
		}
	}
	else
	{
		// Z方向
		for (int i = margin; i < gridZ - margin; i++)
		{
			for (int w = 0; w < riverWidth; w++)
			{
				D3DXVECTOR3 pos(offsetX + (x + w) * areaSize, -48.0f, offsetZ + i * areaSize);

				// 川の生成
				CBlockManager::CreateBlock(CBlock::TYPE_WATER, pos);

				outWaterPositions.push_back(pos);
			}

			x += (rand() % 3) - 1;
			x = std::clamp(x, margin, gridX - margin - riverWidth - 1);
		}
	}
}
//=============================================================================
// クラスタ生成処理
//=============================================================================
void CGenerateMap::GenerateClusters(int gridX, int gridZ, float areaSize,
	float offsetX, float offsetZ, const std::vector<D3DXVECTOR3>& waterPositions)
{
	// クラスター数
	const int clusterCount = 10;

	for (int i = 0; i < clusterCount; i++)
	{
		float centerX = offsetX + (rand() % gridX) * areaSize;
		float centerZ = offsetZ + (rand() % gridZ) * areaSize;
		float radius = 50.0f + rand() % 80;
		int count = 8 + rand() % 3;

		for (int j = 0; j < count; j++)
		{
			D3DXVECTOR3 pos(
				centerX + cosf((rand() % 360) * D3DX_PI / 180.0f) * ((rand() / (float)RAND_MAX) * radius),
				0.0f,
				centerZ + sinf((rand() % 360) * D3DX_PI / 180.0f) * ((rand() / (float)RAND_MAX) * radius)
			);

			// クラスタの要素生成
			CreateClusterElement(pos, areaSize, gridX, gridZ, offsetX, offsetZ, waterPositions);
		}
	}
}
//=============================================================================
// クラスタの要素生成処理
//=============================================================================
void CGenerateMap::CreateClusterElement(const D3DXVECTOR3& pos, float areaSize,
	int gridX, int gridZ, float offsetX, float offsetZ,
	const std::vector<D3DXVECTOR3>& waterPositions)
{
	// --- マップの中心座標を求める ---
	const float mapCenterX = offsetX + (gridX - 1) * areaSize / 2.0f;
	const float mapCenterZ = offsetZ + (gridZ - 1) * areaSize / 2.0f;
	const float halfWidth = (gridX * areaSize) / 2.0f;

	// --- 中心からの距離を計算 ---
	float distX = fabsf(pos.x - mapCenterX);
	float distZ = fabsf(pos.z - mapCenterZ);

	// 外周 → 草
	if (distX > halfWidth * 0.9f || distZ > halfWidth * 0.9f)
	{
		CreateGrassCluster(pos, areaSize, gridX, gridZ, offsetX, offsetZ, waterPositions);
		return;
	}

	CBlock::TYPE type = CBlock::TYPE_GRASS;

	// 水上でなければ生成
	if (!IsCollidingWithWater(pos, areaSize, waterPositions))
	{
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
}
//=============================================================================
// 灯籠補充処理
//=============================================================================
void CGenerateMap::EnsureTorchCount(int gridX, int gridZ, float areaSize,
	float offsetX, float offsetZ, const std::vector<D3DXVECTOR3>& waterPositions,
	std::vector<D3DXVECTOR3>& torchPositions)
{
	const float startX = offsetX + areaSize * 0.5f;
	const float startZ = offsetZ + areaSize * 0.5f;
	const float endX = offsetX + (gridX - 1) * areaSize;
	const float endZ = offsetZ + (gridZ - 1) * areaSize;

	const float INWARD_OFFSET = areaSize * 0.5f; // 壁から離す距離
	const float variation = areaSize * 0.6f;
	auto randVar = [&]()
	{
		return ((rand() / (float)RAND_MAX) - 0.5f) * 2.0f * variation;
	};

	// 4辺からランダムに3つ選ぶ
	std::vector<int> sides = { 0, 1, 2, 3 }; // 0 = 下,1 = 上,2 = 左,3 = 右
	std::shuffle(sides.begin(), sides.end(), std::mt19937{ std::random_device{}() });
	sides.resize(MAX_TORCH);

	for (int side : sides)
	{
		D3DXVECTOR3 pos;

		switch (side)
		{
		case 0: // 下
			pos = { (startX + endX) * 0.8f + randVar(), 0.0f, startZ + INWARD_OFFSET };
			break;
		case 1: // 上
			pos = { (startX + endX) * 0.8f + randVar(), 0.0f, endZ - INWARD_OFFSET };
			break;
		case 2: // 左
			pos = { startX + INWARD_OFFSET, 0.0f, (startZ + endZ) * 0.8f + randVar() };
			break;
		case 3: // 右
			pos = { endX - INWARD_OFFSET, 0.0f, (startZ + endZ) * 0.8f + randVar() };
			break;
		}

		// 水と被ったら
		if (IsCollidingWithWater(pos, areaSize, waterPositions))
		{
			continue;
		}

		if (CBlock* torch = CBlockManager::CreateBlock(CBlock::TYPE_TORCH_01, pos))
		{
			D3DXVECTOR3 offPos = pos;
			offPos.y += 35.0f;
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
	const std::vector<D3DXVECTOR3>& waterPositions, std::vector<D3DXVECTOR3>& treasurePositions)
{
	// 埋蔵金同士の最低距離(クラスター用)
	const float MIN_CLUSTER_DISTANCE = 0.4f * areaSize;

	// 埋蔵金同士の最低距離(通常)
	const float MIN_NORMAL_DISTANCE = 2.0f * areaSize;

	const int TOTAL_TREASURE = MAX_TREASURE;
	const int CLUSTER_MIN = 3;
	const int CLUSTER_MAX = 4;
	const float CLUSTER_RADIUS = areaSize * 0.8f;

	const float SAFE_MARGIN = CLUSTER_RADIUS; // 壁、ブロック余白

	float minX = offsetX + SAFE_MARGIN;
	float maxX = offsetX + (gridX - 1) * areaSize - SAFE_MARGIN;
	float minZ = offsetZ + SAFE_MARGIN;
	float maxZ = offsetZ + (gridZ - 1) * areaSize - SAFE_MARGIN;

	int clusterSize = CLUSTER_MIN + rand() % (CLUSTER_MAX - CLUSTER_MIN + 1);

	// クラスター中心を決める
	D3DXVECTOR3 clusterCenter;
	bool foundCenter = false;

	for (int i = 0; i < MAX_ATTEMPTS && !foundCenter; i++)
	{
		float cx = minX + ((float)rand() / RAND_MAX) * (maxX - minX);
		float cz = minZ + ((float)rand() / RAND_MAX) * (maxZ - minZ);
		D3DXVECTOR3 center(cx, 0.0f, cz);

		// 灯籠と重なっていたら
		if (IsCollidingWithTorch(center, areaSize, torchPositions))
		{
			continue;
		}

		// 水と重なっていたら
		if (IsCollidingWithWater(center, areaSize, waterPositions))
		{
			continue;
		}

		clusterCenter = center;
		foundCenter = true;
	}

	for (int i = 0; i < clusterSize; i++)
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

			if (IsCollidingWithWater(pos, areaSize, waterPositions))
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
				D3DXVECTOR3 offPos = pos;
				offPos.y -= 3.0f;
				treasure->SetPos(offPos);

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
		if (IsCollidingWithWater(pos, areaSize, waterPositions))
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
			D3DXVECTOR3 offPos = treasure->GetPos();
			offPos.y -= 3.0f;
			treasure->SetPos(offPos);

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
// 床を敷き詰める処理
//=============================================================================
void CGenerateMap::FillFloor(int gridX, int gridZ, float areaSize)
{
	float offsetX = -(gridX * areaSize) / 2.0f + areaSize / 2.0f;
	float offsetZ = -(gridZ * areaSize) / 2.0f + areaSize / 2.0f;

	// 既に存在する水ブロックの位置をチェックするためセット作成
	std::vector<D3DXVECTOR3> waterPositions;
	for (auto block : CBlockManager::GetAllBlocks())
	{
		if (block->GetType() == CBlock::TYPE_WATER)
		{
			waterPositions.push_back(block->GetPos());
		}
	}

	// 全マス走査
	for (int x = 0; x < gridX; x++)
	{
		for (int z = 0; z < gridZ; z++)
		{
			D3DXVECTOR3 pos(offsetX + x * areaSize, -48.0f, offsetZ + z * areaSize);

			// 既存水ブロックと重ならない場合のみ床生成
			bool occupied = false;
			for (auto wp : waterPositions)
			{
				if (fabs(wp.x - pos.x) < areaSize * 0.5f && fabs(wp.z - pos.z) < areaSize * 0.5f)
				{
					occupied = true;
					break;
				}
			}

			if (!occupied)
			{
				// 草地面
				CBlockManager::CreateBlock(CBlock::TYPE_GRASS_FLOOR, pos);
			}
			else
			{
				// 土地面
				pos.y = -70.0f;
				CBlockManager::CreateBlock(CBlock::TYPE_SOIL_FLOOR, pos);
			}
		}
	}
}
//=============================================================================
// 外周部に草の連続クラスタを生成
//=============================================================================
void CGenerateMap::GenerateOuterGrassBelt(int gridX, int gridZ, float areaSize,
	float offsetX, float offsetZ,
	const std::vector<D3DXVECTOR3>& waterPositions)
{
	const float startX = offsetX + areaSize * 0.2f;// 内側に少しずらす
	const float startZ = offsetZ + areaSize * 0.2f;
	const float endX = offsetX + (gridX - 1) * areaSize;
	const float endZ = offsetZ + (gridZ - 1) * areaSize;

	const int clusterPerCell = 2 + rand() % 3;		// 1マスあたりの群れの数
	const float step = areaSize / 2.0f;			// 1マス内で複数生成するためのステップ
	const float variation = areaSize * 0.5f;		// ランダムばらつき幅

	auto getVariation = [&]() { return ((rand() / (float)RAND_MAX) - 0.5f) * 2.0f * variation; };

	// 生成する辺をランダムに選ぶ（4辺のうち3つ）
	std::vector<int> sides = { 0, 1, 2, 3 }; // 0=下, 1=上, 2=左, 3=右
	std::random_device rd;
	std::mt19937 g(rd());
	std::shuffle(sides.begin(), sides.end(), g);
	sides.resize(3); // 上位3つだけ残す

	if (std::find(sides.begin(), sides.end(), 0) != sides.end())
	{
		// --- 下辺 ---
		for (float x = startX; x <= endX; x += areaSize / 2.0f)
		{
			for (int i = 0; i < clusterPerCell; ++i)
			{
				D3DXVECTOR3 pos(x + getVariation(), 0.0f, startZ + getVariation());
				if (IsCollidingWithWater(pos, areaSize, waterPositions))
				{// 水と被ったら
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

	if (std::find(sides.begin(), sides.end(), 1) != sides.end())
	{
		// --- 上辺 ---
		for (float x = startX; x <= endX; x += areaSize / 2.0f)
		{
			for (int i = 0; i < clusterPerCell; ++i)
			{
				D3DXVECTOR3 pos(x + getVariation(), 0.0f, endZ + getVariation());
				if (IsCollidingWithWater(pos, areaSize, waterPositions))
				{// 水と被ったら
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

	if (std::find(sides.begin(), sides.end(), 2) != sides.end())
	{
		// --- 左辺 ---
		for (float z = startZ + areaSize / 2.0f; z < endZ; z += areaSize / 2.0f)
		{
			for (int i = 0; i < clusterPerCell; ++i)
			{
				D3DXVECTOR3 pos(startX + getVariation(), 0.0f, z + getVariation());
				if (IsCollidingWithWater(pos, areaSize, waterPositions))
				{// 水と被ったら
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

	if (std::find(sides.begin(), sides.end(), 3) != sides.end())
	{
		// --- 右辺 ---
		for (float z = startZ + areaSize / 2.0f; z < endZ; z += areaSize / 2.0f)
		{
			for (int i = 0; i < clusterPerCell; ++i)
			{
				D3DXVECTOR3 pos(endX + getVariation(), 0.0f, z + getVariation());
				if (IsCollidingWithWater(pos, areaSize, waterPositions))
				{// 水と被ったら
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
	int gridX, int gridZ, float offsetX, float offsetZ,
	const std::vector<D3DXVECTOR3>& waterPositions)
{
	int grassLength = 3 + rand() % 3;    // 草を連続配置する数

	// --- マップ中心を取得 ---
	const float mapCenterX = offsetX + (gridX - 1) * areaSize / 2.0f;
	const float mapCenterZ = offsetZ + (gridZ - 1) * areaSize / 2.0f;
	const float mapHalfX = (gridX * areaSize) / 2.0f;
	const float mapHalfZ = (gridZ * areaSize) / 2.0f;

	// --- 中心からの方向に応じて外向き配置 ---
	D3DXVECTOR3 dir = { 0, 0, 0 };
	if (fabsf(centerPos.x - mapCenterX) > fabsf(centerPos.z - mapCenterZ))
	{
		dir.x = (centerPos.x > mapCenterX) ? 1.0f : -1.0f;
	}
	else
	{
		dir.z = (centerPos.z > mapCenterZ) ? 1.0f : -1.0f;
	}

	for (int k = 0; k < grassLength; k++)
	{
		D3DXVECTOR3 grassPos = centerPos + dir * (k * areaSize);

		// マップ範囲チェック
		if (grassPos.x < offsetX - areaSize || grassPos.x > offsetX + (gridX - 1) * areaSize + areaSize ||
			grassPos.z < offsetZ - areaSize || grassPos.z > offsetZ + (gridZ - 1) * areaSize + areaSize)
		{
			continue;
		}
		// 水と衝突していないかチェック
		if (IsCollidingWithWater(grassPos, areaSize, waterPositions))
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
	outPatrolPoints.clear();

	float offsets[3] = { -gap, 0.0f, gap };

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

	for (int z = 0; z < 3; z++)
	{
		for (int x = 0; x < 3; x++)
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
// 水との重なり判定処理
//=============================================================================
bool CGenerateMap::IsCollidingWithWater(const D3DXVECTOR3& pos, float areaSize, const std::vector<D3DXVECTOR3>& waterPositions)
{
	for (auto& wp : waterPositions)
	{
		if (fabs(wp.x - pos.x) < areaSize * 0.5f &&
			fabs(wp.z - pos.z) < areaSize * 0.5f)
		{
			// 重なった
			return true;
		}
	}

	// 重なっていない
	return false;
}
//=============================================================================
// 灯籠との重なり判定処理
//=============================================================================
bool CGenerateMap::IsCollidingWithTorch(const D3DXVECTOR3& pos, float areaSize, const std::vector<D3DXVECTOR3>& torchPositions)
{
	for (auto& wp : torchPositions)
	{
		if (fabs(wp.x - pos.x) < areaSize * 0.5f &&
			fabs(wp.z - pos.z) < areaSize * 0.5f)
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
	float scaleX = 1.5f + (rand() / (float)RAND_MAX) * 0.8f;
	float scaleY = 1.3f + (rand() / (float)RAND_MAX) * 0.3f;
	float rotY = (rand() % 360) * D3DX_PI / 180.0f;

	block->SetSize(D3DXVECTOR3(scaleX, scaleY, scaleX));
	block->SetRot(D3DXVECTOR3(0.0f, rotY, 0.0f));
}
