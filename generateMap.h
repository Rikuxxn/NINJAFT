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
    // グリッド生成パラメータ
    static constexpr int   GRID_X               = 10;       // Xサイズ
    static constexpr int   GRID_Z               = 10;       // Zサイズ
    static constexpr float AREA_SIZE            = 80.0f;    // 1エリアの広さ

     // 地形
    static constexpr float MAP_SIZE_X           = 800.0f;   // サイズX
    static constexpr float MAP_SIZE_Z           = 800.0f;   // サイズZ
    static constexpr int   MAP_DIV_X            = 100;      // 分割数X
    static constexpr int   MAP_DIV_Z            = 100;      // 分割数Z

    // 水面
    static constexpr float WATER_HEIGHT         = -4.0f;    // 水面の高さ
    static constexpr int   WATER_DIV_X          = 10;       // 分割数X
    static constexpr int   WATER_DIV_Z          = 10;       // 分割数Z

    // 灯籠
    static constexpr int   MAX_TORCH            = 2;        // 灯籠の設置数
    static constexpr float TORCH_OFFSET_HEIGHT  = 38.0f;    // 灯籠の高さのオフセット
    static constexpr float TORCH_INWARD_RATE    = 0.5f;
    static constexpr float TORCH_VARIATION_RATE = 0.6f;
    static constexpr float TORCH_FRONT_X_RATE   = 3.0f;     // 下辺のX中心補正
    static constexpr float TORCH_SIDE_Z_RATE    = 0.8f;     // 左右辺のZ中心補正

    // 灯籠の配置する辺の種類
    enum TorchSide
    {
        TORCH_BOTTOM    = 0,// 下
        TORCH_LEFT      = 1,// 左
        TORCH_RIGHT     = 2,// 右
    };

    // 埋蔵金（距離）
    static constexpr float TREASURE_CLUSTER_MIN_DIST_RATE   = 0.4f;// 最低距離
    static constexpr float TREASURE_NORMAL_MIN_DIST_RATE    = 2.0f;// 離す最低距離

    // 埋蔵金（クラスタ）
    static constexpr int   TREASURE_CLUSTER_MIN             = 3;    // 最低のクラスター数
    static constexpr int   TREASURE_CLUSTER_MAX             = 4;    // 最大のクラスター数
    static constexpr float TREASURE_CLUSTER_RADIUS_RATE     = 0.8f; // クラスターの半径の割合

    // 草
    static constexpr float GRASS_SCALE_X_MIN        = 1.5f;         // 草のスケールXの最低値
    static constexpr float GRASS_SCALE_X_VAR        = 0.8f;         // 草のスケールXの割合
    static constexpr float GRASS_SCALE_Y_MIN        = 1.3f;         // 草のスケールYの最低値
    static constexpr float GRASS_SCALE_Y_VAR        = 0.3f;         // 草のスケールYの割合
    static constexpr int   GRASS_SET_NUM            = 3;			// 草の基本の設置数
    static constexpr int   GRASS_SET_NUM_VAL        = 3;            // 草の増える最大数
    static constexpr float INWARD_OFFSET_RATE       = 0.2f;         // 外周からどれだけ内側に寄せるか
    static constexpr int   MIN_CLUSTER_PER_CELL     = 2;            // 1セルあたりの草クラスター数
    static constexpr int   CLUSTER_PER_CELL_RANGE   = 3;            // 1セルあたりの草クラスター数の増える最大数


    // 草を生成する辺の種類
    enum GrassSide
    {
        GRASS_BOTTOM    = 0,   // 下
        GRASS_TOP       = 1,   // 上
        GRASS_LEFT      = 2,   // 左
        GRASS_RIGHT     = 3,   // 右
        GRASS_MAX
    };

    // 巡回ポイント
    static constexpr float PATROL_GAP           = 210.0f;   // ポイント間の間隔
    static constexpr float PATROL_SAFE_DISTANCE = 120.0f;   // 配置可能距離
    static constexpr int   PATROL_POINT_X       = 3;        // 巡回ポイントのX方向の数
    static constexpr int   PATROL_POINT_Z       = 3;        // 巡回ポイントのZ方向の数

    static constexpr int   MAX_ATTEMPTS         = 50;       // 試行回数
    static constexpr int   MAX_TREASURE         = 8;        // 埋蔵金の設置数
    static constexpr int   CLUSTER_COUNT        = 10;       // クラスター数

     // クラスタ生成
    static constexpr float CLUSTER_RADIUS_MIN   = 50.0f;    // クラスターの最低半径
    static constexpr int   CLUSTER_RADIUS_VAR   = 80;       // クラスターの半径の割合
    static constexpr int   CLUSTER_ELEMENT_MIN  = 8;        // 要素の最低数
    static constexpr int   CLUSTER_ELEMENT_VAR  = 3;        // 要素の追加の最大数

    // マップ境界・中心計算
    static constexpr float HALF_RATE = 0.5f;
    static constexpr float OUTER_GRASS_THRESHOLD_RATE = 0.9f;

private:
    std::vector<D3DXVECTOR3> m_patrolPoints;                // 巡回ポイント
    std::vector<D3DXVECTOR3> m_treasurePositions;           // 埋蔵金の設置ポイント
    CMeshField*              m_pMeshField;                  // メッシュフィールドへのポインタ
    CWaterField*             m_pWater;                      // 水フィールドへのポインタ
};

#endif

