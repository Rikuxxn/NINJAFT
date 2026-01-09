//=============================================================================
//
// テクスチャ処理 [texture.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "texture.h"
#include "renderer.h"
#include "manager.h"

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
int CTexture::m_nNumAll = 0;

//=============================================================================
// コンストラクタ
//=============================================================================
CTexture::CTexture()
{
	// 値のクリア
	for (int nCnt = 0; nCnt < MAX_TEXTURE; nCnt++)
	{
		m_apTexture[nCnt] = {};
	}
}
//=============================================================================
// デストラクタ
//=============================================================================
CTexture::~CTexture()
{
	// なし
}
//=============================================================================
// テクスチャの読み込み
//=============================================================================
HRESULT CTexture::Load(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// 全てのテクスチャの読み込み
	for (int nCnt = 0; nCnt < MAX_TEXTURE; nCnt++)
	{
		if (!TEXTURE[nCnt].empty()) // 空文字列じゃなければロード
		{
			HRESULT hr = D3DXCreateTextureFromFile(
				pDevice,
				TEXTURE[nCnt].c_str(),
				&m_apTexture[nCnt]);

			if (FAILED(hr))
			{
				m_apTexture[nCnt] = nullptr;
			}
		}
		else
		{
			m_apTexture[nCnt] = nullptr;
		}
	}

	return S_OK;
}
//=============================================================================
// テクスチャの破棄
//=============================================================================
void CTexture::Unload(void)
{
	// 全てのテクスチャの破棄
	for (int nCnt = 0; nCnt < MAX_TEXTURE; nCnt++)
	{
		// クリア
		TEXTURE[nCnt].clear();
		if (m_apTexture[nCnt] != nullptr)
		{
			m_apTexture[nCnt]->Release();
			m_apTexture[nCnt] = nullptr;
		}
	}
}
//=============================================================================
// テクスチャの指定
//=============================================================================
int CTexture::Register(const char* pFilename)
{
	for (int nCnt = 0; nCnt < MAX_TEXTURE; nCnt++)
	{
		if (!TEXTURE[nCnt].empty() && TEXTURE[nCnt] == pFilename)
		{
			return nCnt; // 見つかった
		}
	}

	return -1; // 見つからなかった
}
//=============================================================================
// テクスチャの指定
//=============================================================================
int CTexture::RegisterDynamic(const char* pFilename)
{
	// すでにロード済みならインデックス返す
	for (int nCnt = 0; nCnt < m_nNumAll; nCnt++)
	{
		if (!TEXTURE[nCnt].empty() && TEXTURE[nCnt] == pFilename)
		{
			return nCnt;
		}
	}

	// 新しいスロットにロード
	if (m_nNumAll < MAX_TEXTURE)
	{
		// デバイスの取得
		LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

		if (SUCCEEDED(D3DXCreateTextureFromFile(pDevice, pFilename, &m_apTexture[m_nNumAll])))
		{
			TEXTURE[m_nNumAll] = pFilename;  // std::string に代入
			return m_nNumAll++;
		}
	}
	return -1;
}
//=============================================================================
// テクスチャのアドレス取得
//=============================================================================
LPDIRECT3DTEXTURE9 CTexture::GetAddress(int nIdx)
{
	if (nIdx < 0 || nIdx >= MAX_TEXTURE)
	{// 範囲外
		return nullptr;
	}

	return m_apTexture[nIdx];
}