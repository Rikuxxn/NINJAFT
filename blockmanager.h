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
#include "cassert"

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

    void UpdateLight(void) 
    {
        for (const auto& block : m_blocks)
        {
            block->UpdateLight();
        }
    }

    //*****************************************************************************
    // ImGuiサムネイル描画用関数
    //*****************************************************************************
    void ReleaseThumbnailRenderTarget(void);
    HRESULT InitThumbnailRenderTarget(LPDIRECT3DDEVICE9 device);
    IDirect3DTexture9* RenderThumbnail(CBlock* pBlock);
    void GenerateThumbnailsForResources(void);
    IDirect3DTexture9* GetThumbnailTexture(size_t index);

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
            if (!block)
            {
                continue;
            }

            // タイプで判別する
            if (block->GetType() != type)
            {
                continue;
            }

            if (T* t = dynamic_cast<T*>(block))
            {
                result.push_back(t);
            }
        }
        return result;
    }

private:
    static constexpr float THUMB_WIDTH  = 100.0f;// サムネイルの高さ
    static constexpr float THUMB_HEIGHT = 100.0f;// サムネイルの高さ

    //*****************************************************************************
    // ブロック管理
    //*****************************************************************************
    static std::vector<CBlock*> m_blocks;
    static std::unordered_map<CBlock::TYPE, std::vector<CBlock*>> m_blocksByType;

    static CBlock* m_selectedBlock;
    static CBlock* m_draggingBlock;

    static int  m_selectedIdx;
    int         m_prevSelectedIdx = -1;

    bool m_isDragging = false;
    bool m_hasConsumedPayload = false;
    bool m_autoUpdateColliderSize = true;

    //*****************************************************************************
    // デバッグ
    //*****************************************************************************
    CDebugProc3D* m_pDebug3D = nullptr;

    //*****************************************************************************
    // ファイルパス管理
    //*****************************************************************************
    static std::unordered_map<CBlock::TYPE, std::string> s_FilePathMap;

    //*****************************************************************************
    // サムネイル用リソース
    //*****************************************************************************
    LPDIRECT3DTEXTURE9          m_pThumbnailRT;
    LPDIRECT3DSURFACE9          m_pThumbnailZ;
    std::vector<IDirect3DTexture9*> m_thumbnailTextures;
    bool m_thumbnailsGenerated = false;                 // 一度だけ作るフラグ
    float m_thumbWidth;
    float m_thumbHeight;
};

#endif
