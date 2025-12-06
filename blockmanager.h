//=============================================================================
//
// ブロックマネージャー処理 [blockmanager.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _BLOCKMANAGER_H_
#define _BLOCKMANAGER_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "block.h"
#include "debugproc3D.h"
#include <cassert>

//*****************************************************************************
// ブロックマネージャークラス
//*****************************************************************************
class CBlockManager
{
public:
	CBlockManager();
	~CBlockManager();

    static CBlock* CreateBlock(CBlock::TYPE type, D3DXVECTOR3 pos);
    void Init(void);
    void Uninit(void);// 終了処理
    void CleanupDeadBlocks(void);// 削除予約があるブロックの削除
    void Update(void);
    void Draw(void);
    void UpdateInfo(void); // ImGuiでの操作関数をここで呼ぶ用
    void SaveToJson(const char* filename);
    void LoadFromJson(const char* filename);
    void LoadConfig(const std::string& filename);
    void GenerateRandomMap(int seed);
    void GenerateRiver(int GRID_X, int GRID_Z, float AREA_SIZE,
        float offsetX, float offsetZ, std::vector<D3DXVECTOR3>& outWaterPositions);
    void GenerateClusters(int GRID_X, int GRID_Z, float AREA_SIZE,
        float offsetX, float offsetZ, const std::vector<D3DXVECTOR3>& waterPositions,
        std::vector<D3DXVECTOR3>& torchPositions);
    void CreateClusterElement(const D3DXVECTOR3& pos, float AREA_SIZE,
        int GRID_X, int GRID_Z, float offsetX, float offsetZ,
        const std::vector<D3DXVECTOR3>& waterPositions, std::vector<D3DXVECTOR3>& torchPositions,
        int& torchRemaining);
    void EnsureTorchCount(int GRID_X, int GRID_Z, float AREA_SIZE,
        float offsetX, float offsetZ, const std::vector<D3DXVECTOR3>& waterPositions,
        std::vector<D3DXVECTOR3>& torchPositions);
    void EnsureBuriedTreasureCount(int GRID_X, int GRID_Z, float AREA_SIZE,
        float offsetX, float offsetZ, const std::vector<D3DXVECTOR3>& torchPositions,
        const std::vector<D3DXVECTOR3>& waterPositions, std::vector<D3DXVECTOR3>& treasurePositions);
    void CreateGrassCluster(const D3DXVECTOR3& centerPos, float AREA_SIZE,
        int GRID_X, int GRID_Z, float offsetX, float offsetZ,
        const std::vector<D3DXVECTOR3>& waterPositions);
    bool IsCollidingWithWater(const D3DXVECTOR3& pos, float AREA_SIZE, const std::vector<D3DXVECTOR3>& waterPositions);
    bool IsCollidingWithTorch(const D3DXVECTOR3& pos, float AREA_SIZE, const std::vector<D3DXVECTOR3>& torchPositions);
    void ApplyRandomGrassTransform(CBlock* block);
    void FillFloor(int GRID_X, int GRID_Z, float AREA_SIZE);
    void GenerateOuterGrassBelt(int GRID_X, int GRID_Z, float AREA_SIZE,
        float offsetX, float offsetZ, const std::vector<D3DXVECTOR3>& waterPositions);
    void GeneratePatrolPoints(
        const D3DXVECTOR3& origin,   // 中心点
        float gap,                   // ポイント間の距離
        const std::vector<D3DXVECTOR3>& obstaclePositions, // 灯籠や障害物
        float safeDistance,          // 障害物からの最小距離
        std::vector<D3DXVECTOR3>& outPatrolPoints);

    void UpdateLight(void) 
    {
        for (const auto& block : m_blocks)
        {
            block->UpdateLight();
        }
    }

    void ReleaseThumbnailRenderTarget(void);
    HRESULT InitThumbnailRenderTarget(LPDIRECT3DDEVICE9 device);
    IDirect3DTexture9* RenderThumbnail(CBlock* pBlock);
    void GenerateThumbnailsForResources(void);
    IDirect3DTexture9* GetThumbnailTexture(size_t index)
    {
        assert(index < m_thumbnailTextures.size());
        return m_thumbnailTextures[index];
    }

     //*****************************************************************************
    // ImGuiでの操作関数
    //*****************************************************************************
    void UpdateTransform(CBlock* selectedBlock);
    void PickBlockFromMouseClick(void);
    void UpdateCollider(CBlock* selectedBlock);

    //*****************************************************************************
    // getter関数
    //*****************************************************************************
    bool GetUpdateCollider(void) { return m_autoUpdateColliderSize; }
    static const char* GetFilePathFromType(CBlock::TYPE type);
    static std::vector<CBlock*>& GetAllBlocks(void) { return m_blocks; }
    int GetSelectedIdx(void) { return m_selectedIdx; }
    int GetPrevSelectedIdx(void) { return m_prevSelectedIdx; }
    static CBlock* GetSelectedBlock(void) { return m_selectedBlock; }
    bool IsPlayerInGrass(void);
    bool IsPlayerInTorch(void);
    bool IsPlayerInWater(void);
    const std::vector<D3DXVECTOR3>& GetPatrolPoints(void) const { return m_patrolPoints; }

    // 特定のタイプのブロックを取得
    static const std::vector<CBlock*>& GetBlocksByType(CBlock::TYPE type)
    {
        return m_blocksByType[type]; // 存在しない場合は空vectorが返る
    }

    // --- 特定の型をまとめて取得するテンプレート関数 ---
    template<typename T>
    static std::vector<T*> GetBlocksOfType(void)
    {
        std::vector<T*> result;
        CBlock::TYPE type = T::GetStaticType();

        for (CBlock* block : m_blocksByType[type])
        {
            // 実際にインスタンスが T 型か確認
            if (T* t = dynamic_cast<T*>(block))
            {
                if (block->GetType() == type)
                {
                    result.push_back(t);
                }
            }
        }

        return result;
    }

private:
    static std::vector<CBlock*> m_blocks;   // ブロック情報
    static std::unordered_map<CBlock::TYPE, std::vector<CBlock*>> m_blocksByType;
    static CBlock* m_draggingBlock;         // ドラッグ中のブロック情報
    static int m_selectedIdx;               // 選択中のインデックス
    int m_prevSelectedIdx;                  // 前回の選択中のインデックス
    bool m_hasConsumedPayload ;             // ペイロード生成済みフラグ
    CDebugProc3D* m_pDebug3D;			    // 3Dデバッグ表示へのポインタ
    bool m_autoUpdateColliderSize;
    static std::unordered_map<CBlock::TYPE, std::string> s_FilePathMap;
    static CBlock* m_selectedBlock;         // 選択中のブロック
    bool m_isDragging;                      // ドラッグ中かどうか
    std::vector<D3DXVECTOR3> m_patrolPoints;// 巡回ポイント

    LPDIRECT3DTEXTURE9 m_pThumbnailRT;
    LPDIRECT3DSURFACE9  m_pThumbnailZ;

    std::vector<IDirect3DTexture9*> m_thumbnailTextures;
    bool m_thumbnailsGenerated = false;                 // 一度だけ作るフラグ

    float m_thumbWidth = 100.0f;
    float m_thumbHeight = 100.0f;
};

#endif
