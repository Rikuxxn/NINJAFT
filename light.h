//=============================================================================
//
// ライト処理 [light.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _LIGHT_H_// このマクロ定義がされていなかったら
#define _LIGHT_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "objectX.h"

//*****************************************************************************
// ライトクラス
//*****************************************************************************
class CLight
{
private:
    typedef struct
    {
        D3DLIGHT9   light;
        D3DXVECTOR3 direction;
        D3DXVECTOR3 position;
        bool        enabled;
    }LightInfo;

public:
	CLight();
	~CLight();

	HRESULT Init(void);
	static void Uninit(void);
	void Update(void);
	static int AddLight(D3DLIGHTTYPE type, const D3DXCOLOR& diffuse, const D3DXVECTOR3& direction, const D3DXVECTOR3& position);
	void DeleteLight(int index); // ライト削除
    static std::vector<LightInfo> GetCurrentLights(void);
    static void RestoreLights(const std::vector<LightInfo>& backup);
    static int GetLightNum(void) { return m_lightCount; }

private:
    static constexpr int MAX_LIGHTS = 32;       // ライトの最大数

    static LightInfo    m_lights[MAX_LIGHTS];   // ライト情報
    static int          m_lightCount;           // ライトカウント
    static CObjectX*    m_pObjectX;             // Xファイルオブジェクトへのポインタ
};

#endif
