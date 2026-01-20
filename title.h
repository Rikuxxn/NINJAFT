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
	void UpdateLogoVertex(void);
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

private:
	static constexpr int	TITLE_ITEM_NUM	= 1;			// タイトルのUI数
	static constexpr float	ANCHOR_X		= 0.25f;		// 横位置（%）
	static constexpr float	ANCHOR_Y		= 0.3f;			// 縦位置（%）
	static constexpr float	LOGO_WRATE		= 0.4f;			// 画面幅に対しての項目幅率
	static constexpr float	LOGO_HRATE		= 0.3f;			// 画面高さに対しての項目高さ率

	VertexRange					 m_vertexRanges[TYPE_MAX];	// タイプごとに頂点範囲を設定
	LPDIRECT3DVERTEXBUFFER9		 m_pVtxBuff;				// 頂点バッファへのポインタ
	int							 m_nIdxTextureTitle;		// テクスチャインデックス
	static CBlockManager*		 m_pBlockManager;			// ブロックマネージャーへのポインタ
	CLight*						 m_pLight;					// ライトへのポインタ
	int							 m_timer;					// パーティクル生成タイマー
	std::unique_ptr<CItemSelect> m_pItemSelect;				// 項目選択へのポインタ

};

#endif
