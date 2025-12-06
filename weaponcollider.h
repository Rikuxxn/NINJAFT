//=============================================================================
//
// 武器の当たり判定処理 [weaponcollider.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _WEAPONCOLLIDER_H_// このマクロ定義がされていなかったら
#define _WEAPONCOLLIDER_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "collisionUtils.h"
#include "model.h"

// 前方宣言
class CCharacter;

//*****************************************************************************
// 武器コライダークラス
//*****************************************************************************
class CWeaponCollider
{
public:
    CWeaponCollider();
    ~CWeaponCollider();

    void SetActive(bool active) { m_bActive = active; }
    bool IsActive(void) const { return m_bActive; }
    bool IsHit(void) { return m_bHasHit; }
    void ResetHit(void) { m_bHasHit = false; }
    void ResetPrevPos(void) { m_prevTip = m_currTip; m_prevBase = m_currBase; }

    D3DXVECTOR3 GetCurrentBasePos(void) { return m_currBase; }
    D3DXVECTOR3 GetCurrentTipPos(void) { return m_currTip; }

    // 当たり判定の更新処理
    void Update(CModel* pWeapon, float tip, float base)
    {
        // 親子階層込みのワールド行列を取得
        D3DXMATRIX worldMatrix = pWeapon->GetMtxWorld();

        // 刀の根元と先端オフセット（ローカル座標）
        D3DXVECTOR3 localTip(0, tip, 0); // 先端
        D3DXVECTOR3 localBase(0, base, 0); // 根元

        // ローカル→ワールド変換
        D3DXVec3TransformCoord(&m_currTip, &localTip, &worldMatrix);
        D3DXVec3TransformCoord(&m_currBase, &localBase, &worldMatrix);
    }

    // 当たり判定処理
    void CheckHit(CCharacter* pCharacter, float fDamage);

private:
    D3DXVECTOR3 m_prevBase;
    D3DXVECTOR3 m_prevTip;
    D3DXVECTOR3 m_currBase;
    D3DXVECTOR3 m_currTip;
    bool m_bActive;
    bool m_bHasHit;
};

#endif

