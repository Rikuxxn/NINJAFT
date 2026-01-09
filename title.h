//=============================================================================
//
// タイトル処理 [title.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _TITLE_H_// このマクロ定義がされていなかったら
#define _TITLE_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "scene.h"
#include "blockmanager.h"
#include "light.h"
#include "itemselect.h"

//*****************************************************************************
// タイトルクラス
//*****************************************************************************
class CTitle : public CScene
{
public:
	//タイトルの種類
	typedef enum
	{
		TYPE_FIRST = 0,	// タイトル
		TYPE_MAX
	}TYPE;

	CTitle();
	~CTitle();

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	static void ResetLight(void);
	void OnDeviceReset(void) override;
	void ReleaseThumbnail(void) override;
	void ResetThumbnail(void) override;

	static CBlockManager* GetBlockManager(void) { return m_pBlockManager; }

private:

	// 各画像の構造体
	struct ImageInfo
	{
		D3DXVECTOR3 pos;
		float width;
		float height;
	};

	// 頂点の範囲を定義する構造体
	typedef struct
	{
		int start;
		int end;
	}VertexRange;

	// タイプごとに頂点範囲を設定
	VertexRange m_vertexRanges[TYPE_MAX];

	LPDIRECT3DVERTEXBUFFER9		 m_pVtxBuff;		// 頂点バッファへのポインタ
	int							 m_nIdxTextureTitle;// テクスチャインデックス
	static CBlockManager*		 m_pBlockManager;	// ブロックマネージャーへのポインタ
	CLight*						 m_pLight;			// ライトへのポインタ
	int							 m_timer;			// パーティクル生成タイマー
	std::unique_ptr<CItemSelect> m_pItemSelect;		// 項目選択へのポインタ

};

#endif
