//=============================================================================
//
// キャラクターマネージャー処理 [charactermanager.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _CHARACTERMANAGER_H_
#define _CHARACTERMANAGER_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CGuage;

//*****************************************************************************
// キャラクタークラス
//*****************************************************************************
class CCharacter : public CObject
{
public:
    CCharacter(int nPriority = 2);
    virtual ~CCharacter();

    virtual HRESULT Init(void) = 0;
    virtual void Uninit(void) = 0;
    virtual void Update(void) = 0;
    virtual void Draw(void) = 0;

    // 当たり判定の生成
    void CreatePhysics(float radius, float height, btScalar mass);

    // Physicsの破棄
    void ReleasePhysics(void);

    // 当たり判定の位置更新
    void UpdateCollider(D3DXVECTOR3 offset);

    // 向き補間処理
    void UpdateRotation(float fInterpolationSpeed);

    // HPゲージの設定処理
    void SetGuages(D3DXVECTOR3 pos, D3DXCOLOR colHP, D3DXCOLOR colBack, float fWidth, float fHeight);

    // ダメージ処理
    virtual void Damage(float fDamage)
    {
        if (m_isDead)
        {
            return; // すでに死んでいれば処理しない
        }

        m_fHp -= fDamage;

        if (m_fHp <= 0.0f)
        {
            m_isDead = true;// 死んだ
            m_fHp = 0.0f;
        }
    }

    // 回復処理
    void Heal(float fHealAmount)
    {
        m_fHp += fHealAmount;

        if (m_fHp >= m_fMaxHp)
        {
            m_fHp = m_fMaxHp;
        }
    }

    //*****************************************************************************
    // flagment関数
    //*****************************************************************************
    bool IsDead(void) { return m_isDead; }

    //*****************************************************************************
    // setter関数
    //*****************************************************************************
    void SetHp(float fHp) { m_fHp = fHp; m_fMaxHp = m_fHp; }
    void SetPos(D3DXVECTOR3 pos) { m_pos = pos; }
    void SetRot(D3DXVECTOR3 rot) { m_rot = rot; }
    void SetSize(D3DXVECTOR3 size) { m_size = size; }
    void SetRotDest(const D3DXVECTOR3& rotDest) { m_rotDest = rotDest; }
    void SetMove(D3DXVECTOR3 move)
    {
        m_move = move; 

        // リジッドボディに反映
        btVector3 velocity = GetRigidBody()->getLinearVelocity();
        velocity.setX(move.x);
        //velocity.setY(0.0f);
        velocity.setZ(move.z);
        GetRigidBody()->setLinearVelocity(velocity);
    }

    //*****************************************************************************
    // getter関数
    //*****************************************************************************
    D3DXVECTOR3 GetPos(void) { return m_pos; }
    D3DXVECTOR3 GetRot(void) { return m_rot; };
    const D3DXVECTOR3& GetRotDest(void) const { return m_rotDest; }
    D3DXVECTOR3 GetSize(void) { return m_size; }
    D3DXVECTOR3 GetMove(void) const { return m_move; }
    float GetHp(void) const { return m_fHp; }
    float GetMaxHp(void) const { return m_fMaxHp; }
    btScalar GetRadius(void) const { return m_radius; }
    btScalar GetHeight(void) const { return m_height; }
    btRigidBody* GetRigidBody(void) const { return m_pRigidBody; }						// RigidBodyの取得
    btCollisionShape* GetCollisionShape(void) { return m_pShape; }
    D3DXVECTOR3 GetColliderPos(void) const { return m_colliderPos; }
    virtual int GetCollisionFlags(void) const { return 0; }// デフォルトはフラグなし

private:
	float               m_fHp;          // HP
	float               m_fMaxHp;       // HP最大量
    bool                m_isDead;       // 死んだかどうか
	CGuage*             m_pHpGuage;		// 緑ゲージ
	CGuage*             m_pBackGuage;	// 赤ゲージ（遅れて追従）
	CGuage*             m_pFrame;		// 枠
    D3DXVECTOR3         m_pos;			// 位置
    D3DXVECTOR3         m_rot;			// 向き
    D3DXVECTOR3         m_rotDest;		// 向き
    D3DXVECTOR3         m_size;			// サイズ
    D3DXVECTOR3         m_move;			// 移動量
    btRigidBody*        m_pRigidBody;	// 剛体へのポインタ
    btCollisionShape*   m_pShape;	    // 当たり判定の形へのポインタ
    btScalar            m_radius;		// カプセルコライダーの半径
    btScalar            m_height;		// カプセルコライダーの高さ
    D3DXVECTOR3         m_colliderPos;	// カプセルコライダーの位置

};

//*****************************************************************************
// キャラクターマネージャークラス
//*****************************************************************************
class CCharacterManager
{
public:
    // インスタンスの取得
    static CCharacterManager& GetInstance(void)
    {
        static CCharacterManager instance;
        return instance;
    }

    // キャラクター追加処理
    void AddCharacter(CCharacter* pChar)
    {
        // リストに追加する
        m_characters.push_back(pChar);
    }

    // 単体キャラクターの取得処理
    template <class characterType>
    characterType* GetCharacter(void)
    {
        for (const auto& c : m_characters)
        {
            if (auto casted = dynamic_cast<characterType*>(c))
            {
                return casted;
            }
        }
        return nullptr;
    }

    // 複数体キャラクターの取得処理
    template <class characterType>
    std::vector<characterType*> GetCharacters(void)
    {
        std::vector<characterType*> result;
        for (auto* c : m_characters)
        {
            if (auto* casted = dynamic_cast<characterType*>(c))
            {
                result.push_back(casted);
            }
        }
        return result;
    }

    void Init(void)
    {
        for (auto chara : m_characters)
        {
            // キャラクターの初期化処理
            chara->Init();
        }
    }

    void Destroy(void)
    {
        // リストのクリア(空にする)
        m_characters.clear();
    }

    void Update(void)
    {
        for (auto chara : m_characters)
        {
            // キャラクターの更新処理
            chara->Update();
        }
    }

    void Draw(void)
    {
        for (auto chara : m_characters)
        {
            // キャラクターの描画処理
            chara->Draw();
        }
    }

private:
    CCharacterManager() = default;
    ~CCharacterManager() = default;

    CCharacterManager(const CCharacterManager&) = delete;
    CCharacterManager& operator=(const CCharacterManager&) = delete;

    std::vector<CCharacter*> m_characters;// キャラクターのリスト
};

#endif

