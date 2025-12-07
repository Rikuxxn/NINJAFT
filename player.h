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
#include "model.h"
#include "motion.h"
#include "debugproc3D.h"
#include "block.h"
#include "shadowS.h"
#include "effect.h"
#include "state.h"
#include "charactermanager.h"
#include "weaponcollider.h"
#include "game.h"


// 入力データ構造体
struct InputData
{
	D3DXVECTOR3 moveDir;	// 前後移動ベクトル
	bool attack;			// 攻撃入力
	bool stealth;			// 忍び足入力
};

//*****************************************************************************
// プレイヤークラス
//*****************************************************************************
class CPlayer : public CCharacter
{
public:
	CPlayer();
	~CPlayer();

	static constexpr float PLAYER_SPEED = 22.0f;				// 移動時スピード
	static constexpr float STEALTH_SPEED = 8.0f;				// 忍び移動時スピード

	// プレイヤーモーションの種類
	typedef enum
	{
		NEUTRAL = 0,		// 待機
		MOVE,				// 移動
		STEALTH_MOVE,		// 忍び足
		DAMAGE,				// ダメージ
		MAX
	}PLAYER_MOTION;

	static CPlayer* Create(D3DXVECTOR3 pos, D3DXVECTOR3 rot);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	void Respawn(D3DXVECTOR3 pos);

	//*****************************************************************************
	// flagment関数
	//*****************************************************************************
	bool OnGround(btDiscreteDynamicsWorld* world, btRigidBody* playerBody, float rayLength);

	//*****************************************************************************
	// setter関数
	//*****************************************************************************
	void SetInGrass(bool flag) { m_isInGrass = flag; }
	void SetInTorch(bool flag) { m_isInTorch = flag; }
	void SetControlFlag(bool flag) { m_canControl = flag; }

	//*****************************************************************************
	// getter関数
	//*****************************************************************************
	CMotion* GetMotion(void) { return m_pMotion; }
	bool GetOnGround(void) { return m_bOnGround; }
	bool GetIsMoving(void) const { return m_bIsMoving; }
	D3DXVECTOR3 GetForward(void);
	InputData GatherInput(void);
	CBlock* FindFrontBlockByRaycast(float rayLength);
	bool IsStealth(void) { return m_isStealth; }
	CModel** GetModels(void) { return m_apModel; }
	int GetNumModels(void) { return m_nNumModel; }

	// ステート用にフラグ更新
	void UpdateMovementFlags(const D3DXVECTOR3& moveDir)
	{
		m_bIsMoving = (moveDir.x != 0.0f || moveDir.z != 0.0f);
	}

	void Damage(float fDamage) override;

private:
	static constexpr int MAX_PARTS			= 32;		// 最大パーツ数
	static constexpr float CAPSULE_RADIUS	= 10.5f;	// カプセルコライダーの半径
	static constexpr float CAPSULE_HEIGHT	= 45.5f;	// カプセルコライダーの高さ

	D3DXMATRIX m_mtxWorld;				// ワールドマトリックス
	CModel* m_apModel[MAX_PARTS];		// モデル(パーツ)へのポインタ
	CShadowS* m_pShadowS;				// ステンシルシャドウへのポインタ
	CMotion* m_pMotion;					// モーションへのポインタ
	CDebugProc3D* m_pDebug3D;			// 3Dデバッグ表示へのポインタ
	int m_nNumModel;					// モデル(パーツ)の総数
	bool m_bIsMoving;					// 移動入力フラグ
	bool m_bOnGround;					// 接地フラグ
	bool m_isInGrass;					// 草の範囲内か
	bool m_isInTorch;					// 灯籠の範囲内か
	bool m_isStealth;					// ステルス状態か
	bool m_prevIn;						// 直前に入ったか
	int m_deleyTime;					// 移動までの遅延時間(演出等で使用)
	bool m_canControl;					// 操作フラグ
	bool m_smokeActive;					// 煙フラグ
	int m_smokeTimer;					// 煙生成時間

	// ステートを管理するクラスのインスタンス
	StateMachine<CPlayer> m_stateMachine;
};

#endif
