//=============================================================================
//
// テクスチャ処理 [texture.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _TEXTURE_H_// このマクロ定義がされていなかったら
#define _TEXTURE_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************

//*****************************************************************************
// テクスチャクラス
//*****************************************************************************
class CTexture
{
public:
	CTexture();
	~CTexture();

	HRESULT Load(void);
	void Unload(void);
	int Register(const char* pFilename);
	int RegisterDynamic(const char* pFilename);
	LPDIRECT3DTEXTURE9 GetAddress(int nIdx);

private:
	static constexpr int MAX_TEXTURE = 128;// テクスチャの最大数
	std::string TEXTURE[MAX_TEXTURE];
	LPDIRECT3DTEXTURE9 m_apTexture[MAX_TEXTURE];
	static int m_nNumAll;
};

#endif