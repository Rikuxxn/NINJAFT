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

// 前方宣言
class CCharacter;
class CModel;

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
    void Update(CModel* pWeapon, float tip, float base);

    // 当たり判定処理
    void CheckHit(CCharacter* pCharacter, float fDamage);

private:
    D3DXVECTOR3 m_prevBase; // 前回の根元の判定の位置
    D3DXVECTOR3 m_prevTip;  // 前回の先端の判定の位置
    D3DXVECTOR3 m_currBase; // 現在の根元の判定の位置
    D3DXVECTOR3 m_currTip;  // 現在の先端の判定の位置
    bool        m_bActive;  // コライダーONフラグ
    bool        m_bHasHit;  // 当たったかどうか
};

#endif

