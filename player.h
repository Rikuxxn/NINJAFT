//=============================================================================
//
// プレイヤー処理 [player.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _PLAYER_H_// このマクロ定義がされていなかったら
#define _PLAYER_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "state.h"
#include "charactermanager.h"
#include "weaponcollider.h"
#include "blocklist.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CBlock;
class CDebugProc3D;
class CShadowS;
class CMotion;
class CModel;
class CMeshField;

//*****************************************************************************
// プレイヤークラス
//*****************************************************************************
class CPlayer : public CCharacter
{
public:
	CPlayer();
	~CPlayer();

	static constexpr float PLAYER_SPEED			= 22.0f;		// 通常移動時スピード
	static constexpr float INJURY_SPEED			= 12.0f;		// 負傷時スピード
	static constexpr float STEALTH_SPEED		= 8.0f;			// 忍び移動時スピード
	static constexpr float DECELERATION_RATE	= 0.82f;		// 減速率
	static constexpr float ACCEL_RATE			= 0.15f;		// スピードの補間率

	// プレイヤーモーションの種類
	typedef enum
	{
		NEUTRAL = 0,		// 待機
		MOVE,				// 移動
		INJURY,				// 負傷
		STEALTH_MOVE,		// 忍び足
		DAMAGE,				// ダメージ
		START,				// スタート時
		DIG_UP,				// 掘り出し
		MAX
	}PLAYER_MOTION;

	// 入力データ構造体
	struct InputData
	{
		D3DXVECTOR3 moveDir;	// 前後移動ベクトル
		bool attack;			// 攻撃入力
		bool stealth;			// 忍び足入力
	};

	static CPlayer* Create(D3DXVECTOR3 pos, D3DXVECTOR3 rot);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	void Respawn(D3DXVECTOR3 pos);
	void ApplyDeceleration(void)
	{
		D3DXVECTOR3 move = GetMove();

		// 減速させる
		move *= DECELERATION_RATE;

		if (fabsf(move.x) < 0.01f) move.x = 0;
		if (fabsf(move.z) < 0.01f) move.z = 0;

		// 移動量の設定
		SetMove(move);
	}

	//*****************************************************************************
	// flagment関数
	//*****************************************************************************
	bool OnGroundMesh(const CMeshField* field, float footOffset);

	//*****************************************************************************
	// setter関数
	//*****************************************************************************
	void SetInGrass(bool flag) { m_isInGrass = flag; }
	void SetInTorch(bool flag) { m_isInTorch = flag; }
	void SetControlFlag(bool flag) { m_canControl = flag; }
	void SetIsDead(bool flag) { m_isDead = flag; }
	void SetOnGround(bool flag) { m_bOnGround = flag; }
	void SetDamagePhysics(bool flag) { m_bDamagePhysics = flag; }
	void SetTargetTreasure(CBuriedTreasureBlock* p) { m_pTargetTreasure = p; }

	//*****************************************************************************
	// getter関数
	//*****************************************************************************
	CMotion* GetMotion(void) const { return m_pMotion; }
	bool GetOnGround(void) { return m_bOnGround; }
	bool GetIsMoving(void) const { return m_bIsMoving; }
	D3DXVECTOR3 GetForward(void);
	InputData GatherInput(void);
	CBlock* FindFrontBlockByRaycast(float rayLength);
	bool IsStealth(void) const { return m_isStealth; }
	CModel** GetModels(void) { return m_apModel; }
	int GetNumModels(void) { return m_nNumModel; }
	bool GetControlFlag(void) { return m_canControl; }
	CBuriedTreasureBlock* GetTargetTreasure(void) const { return m_pTargetTreasure; }

	// ステート用にフラグ更新
	void UpdateMovementFlags(const D3DXVECTOR3& moveDir)
	{
		m_bIsMoving = (moveDir.x != 0.0f || moveDir.z != 0.0f);
	}

	void Damage(float fDamage) override;

private:
	static constexpr int	MAX_PARTS			= 32;		// 最大パーツ数
	static constexpr float	CAPSULE_RADIUS		= 10.5f;	// カプセルコライダーの半径
	static constexpr float	CAPSULE_HEIGHT		= 45.5f;	// カプセルコライダーの高さ
	static constexpr float	RESPAWN_HEIGHT		= -280.0f;	// リスポーンする高さ
	static constexpr float	COLLIDER_OFFSET		= 35.0f;	// コライダーオフセット位置
	static constexpr int	EFFECT_CREATE_NUM	= 3;		// エフェクト生成数
	static constexpr float	HEIGHT_STEP			= 30.0f;	// 高さの増加量

	D3DXMATRIX				m_mtxWorld;						// ワールドマトリックス
	CModel*					m_apModel[MAX_PARTS];			// モデル(パーツ)へのポインタ
	CShadowS*				m_pShadowS;						// ステンシルシャドウへのポインタ
	CMotion*				m_pMotion;						// モーションへのポインタ
	CDebugProc3D*			m_pDebug3D;						// 3Dデバッグ表示へのポインタ
	CBuriedTreasureBlock*	m_pTargetTreasure;				// 埋蔵金ブロックへのポインタ
	int						m_nNumModel;					// モデル(パーツ)の総数
	int						m_smokeTimer;					// 煙生成時間
	int						m_deleyTime;					// 移動までの遅延時間(演出等で使用)
	bool					m_bIsMoving;					// 移動入力フラグ
	bool					m_bOnGround;					// 接地フラグ
	bool					m_isInGrass;					// 草の範囲内か
	bool					m_isInTorch;					// 灯籠の範囲内か
	bool					m_isStealth;					// ステルス状態か
	bool					m_canControl;					// 操作フラグ
	bool					m_smokeActive;					// 煙フラグ
	bool					m_isGameStartSmoke;				// ゲーム開始フラグ
	bool					m_isDead;						// 死亡したか
	bool					m_bDamagePhysics;				// ダメージ時に重力を使うか

	// ステートを管理するクラスのインスタンス
	StateMachine<CPlayer> m_stateMachine;
};

#endif
