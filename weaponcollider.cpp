//=============================================================================
//
// 武器の当たり判定処理 [weaponcollider.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "weaponcollider.h"
#include "enemy.h"
#include "player.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CWeaponCollider::CWeaponCollider()
{
	// 値のクリア
	m_prevBase = INIT_VEC3;
	m_prevTip = INIT_VEC3;
	m_currBase = INIT_VEC3;
	m_currTip = INIT_VEC3;
	m_bActive = false;
	m_bHasHit = false;
}
//=============================================================================
// デストラクタ
//=============================================================================
CWeaponCollider::~CWeaponCollider()
{
	// なし
}
//=============================================================================
// 当たり判定の更新処理
//=============================================================================
void CWeaponCollider::Update(CModel* pWeapon, float tip, float base)
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
//=============================================================================
// 当たり判定処理
//=============================================================================
void CWeaponCollider::CheckHit(CCharacter* pCharacter, float fDamage)
{
    if (!m_bActive || m_bHasHit || !pCharacter)
    {
        return;
    }

    // カプセルの上下端点
    D3DXVECTOR3 top = pCharacter->GetPos() + D3DXVECTOR3(0, pCharacter->GetHeight() * 0.5f, 0);
    D3DXVECTOR3 bottom = pCharacter->GetPos() - D3DXVECTOR3(0, pCharacter->GetHeight() * 0.5f, 0);
    float radius = pCharacter->GetRadius();

    float weaponRadius = 30.0f; // 当たり判定の半径

    if (CCollision::IntersectSegmentCapsule(m_prevBase, m_currBase, bottom, top, radius + weaponRadius) ||
        CCollision::IntersectSegmentCapsule(m_prevTip, m_currTip, bottom, top, radius + weaponRadius) ||
        CCollision::IntersectSegmentCapsule(m_prevBase, m_currTip, bottom, top, radius + weaponRadius))
    {
        // ダメージ処理
        pCharacter->Damage(fDamage);
        m_bHasHit = true; // 当たった
    }

    // 座標更新
    m_prevBase = m_currBase;
    m_prevTip = m_currTip;
}
