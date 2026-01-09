//=============================================================================
//
// キャラクターマネージャー処理 [charactermanager.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "charactermanager.h"
#include "manager.h"
#include "guage.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CCharacter::CCharacter(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	m_fHp           = 0.0f;                         // HP
	m_fMaxHp        = 0.0f;                         // HP最大量
	m_pHpGuage      = nullptr;		                // 緑ゲージ
	m_pBackGuage    = nullptr;		                // 赤ゲージ（遅れて追従）
	m_pFrame        = nullptr;			            // 枠
	m_pos           = INIT_VEC3;					// 位置
	m_rot           = INIT_VEC3;					// 向き
	m_rotDest       = INIT_VEC3;					// 向き
	m_size          = D3DXVECTOR3(1.0f, 1.0f, 1.0f);// サイズ
    m_move          = INIT_VEC3;					// 移動量
	m_pRigidBody    = nullptr;						// 剛体へのポインタ
	m_pShape        = nullptr;						// 当たり判定の形へのポインタ
	m_radius        = 0.0f;							// カプセルコライダーの半径
	m_height        = 0.0f;							// カプセルコライダーの高さ
	m_colliderPos   = INIT_VEC3;					// コライダーの位置

}
//=============================================================================
// デストラクタ
//=============================================================================
CCharacter::~CCharacter()
{
	// なし
}
//=============================================================================
// 当たり判定の生成処理
//=============================================================================
void CCharacter::CreatePhysics(float radius, float height, btScalar mass)
{
    //*********************************************************************
    // Bullet Physics カプセルコライダーの設定
    //*********************************************************************

    m_radius = radius;
    m_height = height;

    // コライダーを生成(カプセル)
    m_pShape = new btCapsuleShape(m_radius, m_height);

    // コライダー中心 = 足元 + オフセット
    m_colliderPos = GetPos() + D3DXVECTOR3(0, 20.0f, 0);

    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(btVector3(m_colliderPos.x, m_colliderPos.y, m_colliderPos.z));

    // 質量を設定
    btVector3 inertia(0, 0, 0);  // 慣性

    m_pShape->calculateLocalInertia(mass, inertia);

    btDefaultMotionState* motionState = new btDefaultMotionState(transform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, m_pShape, inertia);

    m_pRigidBody = new btRigidBody(rbInfo);

    int flags = m_pRigidBody->getCollisionFlags();
    flags |= GetCollisionFlags();

    m_pRigidBody->setCollisionFlags(flags);					// コリジョンフラグ
    m_pRigidBody->setAngularFactor(btVector3(0, 0, 0));     // 移動方向
    m_pRigidBody->setFriction(0.0f);                        // 摩擦
    m_pRigidBody->setRollingFriction(0.0f);                 // 転がり摩擦

    m_pRigidBody->setUserPointer(this);
    m_pRigidBody->setActivationState(DISABLE_DEACTIVATION);// スリープ状態にしない

    // 物理ワールドに追加
    btDiscreteDynamicsWorld* pWorld = CManager::GetPhysicsWorld();

    if (pWorld != nullptr)
    {
        pWorld->addRigidBody(m_pRigidBody);
    }
}
//=============================================================================
// Physicsの破棄
//=============================================================================
void CCharacter::ReleasePhysics(void)
{
    // 剛体の削除
    if (m_pRigidBody)
    {
        // まず、物理ワールドから削除
        btDiscreteDynamicsWorld* pWorld = CManager::GetPhysicsWorld();
        if (pWorld)
        {
            pWorld->removeRigidBody(m_pRigidBody);
        }

        // 剛体の破棄
        delete m_pRigidBody->getMotionState();
        delete m_pRigidBody;
        m_pRigidBody = nullptr;
    }

    // コライダーの破棄
    if (m_pShape)
    {
        delete m_pShape;
        m_pShape = nullptr;
    }
}
//=============================================================================
// 当たり判定の位置更新
//=============================================================================
void CCharacter::UpdateCollider(D3DXVECTOR3 offset)
{
    // 現在位置を物理ワールドから取得して m_pos に反映
    btTransform trans;
    m_pRigidBody->getMotionState()->getWorldTransform(trans);
    btVector3 pos = trans.getOrigin();
    m_colliderPos = D3DXVECTOR3(pos.getX(), pos.getY(), pos.getZ());
    m_pos = m_colliderPos - offset; // 足元へのオフセット
}
//=============================================================================
// 向き補間処理
//=============================================================================
void CCharacter::UpdateRotation(float fInterpolationSpeed)
{
    // Y軸の差分が180度を超える場合は補正
    if (m_rotDest.y - m_rot.y > D3DX_PI)
    {
        m_rot.y += D3DX_PI * 2.0f;
    }
    else if (m_rotDest.y - m_rot.y < -D3DX_PI)
    {
        m_rot.y -= D3DX_PI * 2.0f;
    }

    // 補間して滑らかに回転
    m_rot.y += (m_rotDest.y - m_rot.y) * fInterpolationSpeed;
}
//=============================================================================
// HPゲージの設定処理
//=============================================================================
void CCharacter::SetGuages(D3DXVECTOR3 pos, D3DXCOLOR colHP, D3DXCOLOR colBack, float fWidth, float fHeight)
{
    m_pFrame = CGuage::Create(CGuage::TYPE_FRAME, pos, fWidth, fHeight);// 枠
    m_pBackGuage = CGuage::Create(CGuage::TYPE_BACKGUAGE, pos, fWidth, fHeight);// バックゲージ
    m_pHpGuage = CGuage::Create(CGuage::TYPE_GUAGE, pos, fWidth, fHeight);// HPゲージ

    // HPを個別管理するためにゲージのターゲットを設定
    m_pFrame->SetTargetCharacter(this);
    m_pBackGuage->SetTargetCharacter(this);
    m_pHpGuage->SetTargetCharacter(this);

    // 色の設定
    m_pHpGuage->SetCol(colHP);
    m_pBackGuage->SetCol(colBack);
}
