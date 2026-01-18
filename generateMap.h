//=============================================================================
//
// マップ生成処理 [generateMap.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _GENERATEMAP_H_
#define _GENERATEMAP_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************


//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CBlock;
class CMeshField;
class CWaterField;

//*****************************************************************************
// マップジェネレータークラス
//*****************************************************************************
class CGenerateMap
{
public:
    CGenerateMap() {}
    ~CGenerateMap() {}

    // インスタンス生成
    static CGenerateMap* GetInstance(void);

    // リストクリア処理
    void Uninit(void)
    {
        // リストのクリア
        m_patrolPoints.clear();
        m_treasurePositions.clear();
    }

    // マップランダム生成処理(集約)
    void GenerateRandomMap(int seed);

    // 地形ランダム生成
    void GenerateRandomTerrain(int seed);

    void GenerateClusters(int gridX, int gridZ, float areaSize,
        float offsetX, float offsetZ);
    void CreateClusterElement(const D3DXVECTOR3& pos, float AREA_SIZE,
        int GRID_X, int GRID_Z, float offsetX, float offsetZ);
    void EnsureTorchCount(int gridX, int gridZ, float areaSize,
        float offsetX, float offsetZ, std::vector<D3DXVECTOR3>& torchPositions);
    void EnsureBuriedTreasureCount(int gridX, int gridZ, float areaSize,
        float offsetX, float offsetZ, const std::vector<D3DXVECTOR3>& torchPositions,
        std::vector<D3DXVECTOR3>& treasurePositions);
    void CreateGrassCluster(const D3DXVECTOR3& centerPos, float areaSize,
        int gridX, int gridZ, float offsetX, float offsetZ);
    bool IsCollidingWithTorch(const D3DXVECTOR3& pos, float areaSize, const std::vector<D3DXVECTOR3>& torchPositions);
    void ApplyRandomGrassTransform(CBlock* block);
    void GenerateOuterGrassBelt(int gridX, int gridZ, float areaSize, float offsetX, float offsetZ);
    void GeneratePatrolPoints(
        const D3DXVECTOR3& origin,   // 中心点
        float gap,                   // ポイント間の距離
        const std::vector<D3DXVECTOR3>& obstaclePositions, // 灯籠や障害物
        float safeDistance,          // 障害物からの最小距離
        std::vector<D3DXVECTOR3>& outPatrolPoints);
    void OnTreasureCollected(const D3DXVECTOR3& pos);

    const std::vector<D3DXVECTOR3>& GetPatrolPoints(void) const { return m_patrolPoints; }
    const std::vector<D3DXVECTOR3>& GetTreasurePositions(void) const { return m_treasurePositions; }

    CMeshField* GetMeshField(void) { return m_pMeshField; }
    CWaterField* GetWaterField(void) { return m_pWater; }

private:
    static constexpr int MAX_ATTEMPTS   = 50;       // 試行回数
    static constexpr int MAX_TORCH      = 2;        // 灯籠の設置数
    static constexpr int MAX_TREASURE   = 8;        // 埋蔵金の設置数

    // マップ生成パラメータ
    static constexpr int    GRID_X         = 10;    // Xサイズ
    static constexpr int    GRID_Z         = 10;    // Zサイズ
    static constexpr float  AREA_SIZE      = 80.0f; // 1エリアの広さ

    std::vector<D3DXVECTOR3> m_patrolPoints;        // 巡回ポイント
    std::vector<D3DXVECTOR3> m_treasurePositions;   // 埋蔵金の設置ポイント
    CMeshField* m_pMeshField;                       // メッシュフィールドへのポインタ
    CWaterField* m_pWater;                          // 水フィールドへのポインタ
};

#endif

