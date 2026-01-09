//=============================================================================
//
// レイキャスト処理 [raycast.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "raycast.h"
#include "renderer.h"
#include "manager.h"

// 名前空間stdの使用
using namespace std;

//=============================================================================
// マウス位置からワールドレイを取得する処理
//=============================================================================
bool CRayCast::GetMouseRay(D3DXVECTOR3& outOrigin, D3DXVECTOR3& outDir)
{
    // マウス座標取得
    POINT pt;
    GetCursorPos(&pt);

    // クライアント座標に変換
    HWND hwnd = GetActiveWindow();
    ScreenToClient(hwnd, &pt);

    // デバイス・ビューポート
    CRenderer* renderer = CManager::GetRenderer();
    LPDIRECT3DDEVICE9 pDevice = renderer->GetDevice();

    D3DVIEWPORT9 vp;
    pDevice->GetViewport(&vp);

    // 行列取得
    D3DXMATRIX matProj, matView, matWorld;
    pDevice->GetTransform(D3DTS_PROJECTION, &matProj);
    pDevice->GetTransform(D3DTS_VIEW, &matView);
    D3DXMatrixIdentity(&matWorld); // 単位行列

    // Unprojectで near/far の2点をワールド空間に変換
    D3DXVECTOR3 nearPt((float)pt.x, (float)pt.y, 0.0f);
    D3DXVECTOR3 farPt((float)pt.x, (float)pt.y, 1.0f);

    D3DXVECTOR3 worldNear, worldFar;
    D3DXVec3Unproject(&worldNear, &nearPt, &vp, &matProj, &matView, &matWorld);
    D3DXVec3Unproject(&worldFar, &farPt, &vp, &matProj, &matView, &matWorld);

    // レイの出発点と方向
    outOrigin = worldNear;
    outDir = worldFar - worldNear;
    D3DXVec3Normalize(&outDir, &outDir);

    return true;
}
//=============================================================================
// レイとAABBの当たり判定
//=============================================================================
bool CRayCast::IntersectAABB(const D3DXVECTOR3& rayOrigin,
    const D3DXVECTOR3& rayDir,
    const D3DXVECTOR3& boxMin,
    const D3DXVECTOR3& boxMax,
    float& outDist)
{
    const float EPSILON = 1e-6f;
    float tMin = 0.0f;
    float tMax = FLT_MAX;

    for (int i = 0; i < 3; ++i)
    {
        float rayOrig = ((float*)&rayOrigin)[i];
        float rayDirI = ((float*)&rayDir)[i];
        float bMin = ((float*)&boxMin)[i];
        float bMax = ((float*)&boxMax)[i];

        if (fabsf(rayDirI) < EPSILON)
        {
            if (rayOrig < bMin || rayOrig > bMax)
            {
                return false;
            }
        }
        else
        {
            float invDir = 1.0f / rayDirI;
            float t1 = (bMin - rayOrig) * invDir;
            float t2 = (bMax - rayOrig) * invDir;

            if (t1 > t2)
            {
                std::swap(t1, t2);
            }

            tMin = max(tMin, t1);
            tMax = min(tMax, t2);

            if (tMin > tMax)
            {
                return false;
            }
        }
    }

    outDist = tMin;
    return true;
}
//=============================================================================
// レイとOBBの当たり判定
//=============================================================================
bool CRayCast::IntersectOBB(
    const D3DXVECTOR3& rayOrigin,
    const D3DXVECTOR3& rayDir,
    const D3DXMATRIX& worldMatrix,
    const D3DXVECTOR3& halfSize,
    float& outDistance
)
{
    // ワールド→ローカルへの逆行列を計算
    D3DXMATRIX invWorld;
    if (D3DXMatrixInverse(&invWorld, NULL, &worldMatrix) == NULL)
    {
        return false;
    }

    // レイをローカル空間に変換
    D3DXVECTOR3 localOrigin, localDir;
    D3DXVec3TransformCoord(&localOrigin, &rayOrigin, &invWorld);
    D3DXVec3TransformNormal(&localDir, &rayDir, &invWorld);
    D3DXVec3Normalize(&localDir, &localDir);

    // ローカル空間のAABBに対するレイ当たり判定
    // AABBのmin/maxを計算
    D3DXVECTOR3 boxMin = -halfSize;
    D3DXVECTOR3 boxMax = +halfSize;

    float tmin = -FLT_MAX;
    float tmax = FLT_MAX;

    // 各軸ごとにスラブ法（Slab method）で交差計算
    for (int i = 0; i < 3; i++)
    {
        float origin = ((float*)&localOrigin)[i];
        float dir = ((float*)&localDir)[i];
        float min = ((float*)&boxMin)[i];
        float max = ((float*)&boxMax)[i];

        if (fabsf(dir) < 1e-6f)
        {
            if (origin < min || origin > max)
            {
                return false;
            }
        }
        else
        {
            float t1 = (min - origin) / dir;
            float t2 = (max - origin) / dir;

            if (t1 > t2)
            {
                std::swap(t1, t2);
            }
            tmin = std::max(tmin, t1);
            tmax = std::min(tmax, t2);

            if (tmin > tmax)
            {
                return false;
            }
        }
    }

    // 最小交差距離を返す

    if (tmin < 0.0f)
    {
        return false; // レイの始点より後ろの交差は無効
    }

    outDistance = tmin;
    return true;
}
