//=============================================================================
//
// 敵の処理 [enemy.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _ENEMY_H_
#define _ENEMY_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "model.h"
#include "motion.h"
#include "state.h"
#include "debugproc3D.h"
#include "charactermanager.h"
#include "game.h"
#include "enemyAI.h"
#include "player.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CEnemyLeader;
class CEnemySub;
class CWeaponCollider;

//*****************************************************************************
// 敵クラス
//*****************************************************************************
class CEnemy : public CCharacter
{
public:
	CEnemy();
	~CEnemy();

	// AIが要求する行動タイプ
	typedef enum
	{
		ACTION_NONE,

		// 共通
		AI_NEUTRAL,				// 待機
		AI_MOVE,				// 移動
		AI_SOUND_INVESTIGATE,	// 音源の調査
		AI_CAUTION,				// 警戒
		AI_CHASE,				// プレイヤー追跡

		// リーダー
		AI_TREASURE_INVESTIGATE,// 埋蔵金の調査
		AI_CLOSE_ATTACK_01,		// 近距離攻撃1
		AI_CLOSE_ATTACK_02,		// 近距離攻撃2
		AI_DOUBT,				// 疑い
		AI_ORDER,				// 命令
		AI_DISCOVER,			// 発見

		// サブ
		AI_FOLLOW,				// リーダー追従

		AI_MAX
	}EEnemyAction;

	template <class T>
	static  T* CreateTyped(D3DXVECTOR3 pos, D3DXVECTOR3 rot)
	{
		static_assert(std::is_base_of<CEnemy, T>::value, "T must inherit CEnemy");

		T* pEnemy = new T();

		// nullptrだったら
		if (pEnemy == nullptr)
		{
			return nullptr;
		}

		pEnemy->SetPos(pos);
		pEnemy->SetRot(rot);
		pEnemy->SetSize(D3DXVECTOR3(1.1f, 1.1f, 1.1f));

		// 初期化失敗時
		if (FAILED(pEnemy->Init()))
		{
			return nullptr;
		}

		return pEnemy;
	}

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);
	void OnSoundHeard(const D3DXVECTOR3& soundPos)
	{ 
		m_lastHeardSoundPos = soundPos; 
		m_hasHeardSound = true;
	}

	void ChooseNextPatrolPoint(void)  // 隣接するポイントからランダムに次のポイント選択
	{
		if (m_patrolPoints.empty())
		{
			return;
		}

		// 現在のポイントのインデックスを求める
		int currentIndex = -1;
		for (size_t nCnt = 0; nCnt < m_patrolPoints.size(); ++nCnt)
		{
			if (m_patrolPoints[nCnt] == m_currentPatrolTarget)
			{
				currentIndex = (int)nCnt;
				break;
			}
		}

		if (currentIndex == -1)
		{
			return;
		}

		// 3x3なので x,z を計算
		int x = currentIndex % 3;
		int z = currentIndex / 3;

		// 隣接ポイントの候補を作る
		std::vector<int> candidates;
		for (int dz = -1; dz <= 1; ++dz)
		{
			for (int dx = -1; dx <= 1; ++dx)
			{
				int nx = x + dx;
				int nz = z + dz;

				// 自分自身は除外
				if (dx == 0 && dz == 0)
				{
					continue;
				}

				// 3x3範囲内かチェック
				if (nx < 0 || nx >= 3 || nz < 0 || nz >= 3)
				{
					continue;
				}

				int newIndex = nz * 3 + nx;
				candidates.push_back(newIndex);
			}
		}

		int r = rand() % m_patrolPoints.size();
		m_currentPatrolTarget = m_patrolPoints[r];
	}

	// 一番近い巡回ポイントに戻す処理
	void ReturnToPatrol(void)
	{
		float minDist = FLT_MAX;
		int closestIndex = 0;

		for (size_t nCnt = 0; nCnt < m_patrolPoints.size(); ++nCnt)
		{
			D3DXVECTOR3 dis = m_patrolPoints[nCnt] - GetPos();

			// 一番近い巡回ポイントに設定する
			float dist = D3DXVec3Length(&dis);
			if (dist < minDist)
			{
				minDist = dist;
				closestIndex = (int)nCnt;
			}
		}

		m_currentPatrolTarget = m_patrolPoints[closestIndex];
		m_returnToPatrol = false;  // 巡回再開準備完了
		m_hasHeardSound = false;   // 音源処理終了
	}

	// 一番近い埋蔵金ポイントの設定処理
	void SetNearestTreasurePosition(void);

	// 巡回ポイントに到達したか
	bool HasReachedTarget(void) // 到達判定
	{
		D3DXVECTOR3 pos = GetPos();
		D3DXVECTOR3 dis = m_currentPatrolTarget - pos;
		return D3DXVec3Length(&dis) < 60.0f; // 到達距離
	}

	// 埋蔵金の位置に到達したか
	bool HasReachedTreasure(void)
	{
		D3DXVECTOR3 pos = GetPos();
		D3DXVECTOR3 dis = m_nearestTreasurePosition - pos;
		return D3DXVec3Length(&dis) < 60.0f; // 到達距離
	}

	// 音源の位置に到達したか
	bool HasReachedSoundTarget(void) // 到達判定
	{
		D3DXVECTOR3 pos = GetPos();
		D3DXVECTOR3 dis = m_lastHeardSoundPos - pos;
		return D3DXVec3Length(&dis) < 60.0f; // 到達距離
	}

	bool IsPlayerInSight(CPlayer* pPlayer);
	bool IsSubAction(EEnemyAction type);
	bool IsLeaderAction(EEnemyAction type);
	bool OnGroundMesh(const CMeshField* field, float footOffset);

	//*****************************************************************************
	// setter関数
	//*****************************************************************************
	void SetRequestedAction(EEnemyAction action) { m_requestedAction = action; }
	void SetPatrolPoints(const std::vector<D3DXVECTOR3>& points)
	{
		m_patrolPoints = points;
	}
	void SetupModels(CModel** pModels, int nNumModels)
	{
		m_nNumModel = nNumModels;

		for (int nCnt = 0; nCnt < nNumModels; nCnt++)
		{
			m_apModel[nCnt] = pModels[nCnt];

			m_apModel[nCnt]->SetOffsetPos(m_apModel[nCnt]->GetPos());
			m_apModel[nCnt]->SetOffsetRot(m_apModel[nCnt]->GetRot());

			if (strstr(m_apModel[nCnt]->GetPath(), "weapon") != nullptr)
			{
				m_pSwordModel = m_apModel[nCnt];
			}
		}
	}
	void SetSightRange(float range) { m_sightRange = range; }
	void SetSightAngle(float angle) { m_sightAngle = angle; }
	void SetAI(std::unique_ptr<IEnemyAI> ai) { m_pAI = std::move(ai); }
	void SetControlFlag(bool flag) { m_canControl = flag; }
	void SetSoundCount(int nCount) { m_makeSoundCount = nCount; }
	void SetInsightCount(int nCount) { m_insightCount = nCount; }

	//*****************************************************************************
	// getter関数
	//*****************************************************************************
	CModel* GetWeapon(void) { return m_pSwordModel; }
	D3DXVECTOR3 GetForward(void);
	EEnemyAction GetRequestedAction(void) const { return m_requestedAction; }
	D3DXVECTOR3 GetPatrolTarget(void) const { return m_currentPatrolTarget; }
	D3DXVECTOR3 GetNearestTreasurePos(void) const { return m_nearestTreasurePosition; }
	D3DXVECTOR3 GetLastHeardSoundPos(void) const { return m_lastHeardSoundPos; }
	bool HasHeardSound(void) { return m_hasHeardSound; }
	bool IsInvestigating(void) { return m_returnToPatrol; }
	CModel** GetModels(void) { return m_apModel; }
	int GetNumModels(void) { return m_nNumModel; }
	IEnemyAI* GetAI(void) { return m_pAI.get(); }
	bool GetControlFlag(void) { return m_canControl; }
	int GetSoundCount(void) { return m_makeSoundCount; }
	int GetInsightCount(void) { return m_insightCount; }

private:
	static constexpr int	MAX_PARTS				= 32;		// 最大パーツ数
	static constexpr float	CAPSULE_RADIUS			= 14.0f;	// カプセルコライダーの半径
	static constexpr float	CAPSULE_HEIGHT			= 60.0f;	// カプセルコライダーの高さ
	static constexpr float	DEFAULT_COLLIDER_OFFSET = 45.0f;	// コライダーのオフセット位置

	D3DXMATRIX					m_mtxWorld;						// ワールドマトリックス
	CModel*						m_apModel[MAX_PARTS];			// モデル(パーツ)へのポインタ
	CDebugProc3D*				m_pDebug3D;						// 3Dデバッグ表示へのポインタ
	int							m_nNumModel;					// モデル(パーツ)の総数
	CModel*						m_pSwordModel;					// 武器モデルのポインタ
	float						m_sightRange;					// 視界距離
	float						m_sightAngle;					// 視界範囲
	EEnemyAction				m_requestedAction;				// AIのリクエスト
	std::vector<D3DXVECTOR3>	m_patrolPoints;					// 巡回ポイント
	D3DXVECTOR3					m_currentPatrolTarget;			// 現在の巡回ポイント
	D3DXVECTOR3					m_nearestTreasurePosition;		// 一番近い埋蔵金ポイント
	D3DXVECTOR3					m_lastHeardSoundPos;			// 最後に聞いた音の座標
	bool						m_hasHeardSound;				// 音を聞いたかどうか
	bool						m_returnToPatrol;				// 最寄りの巡回ポイントに戻るフラグ
	bool						m_canControl;					// 操作フラグ
	int							m_makeSoundCount;				// 音発生数
	int							m_insightCount;					// 発見された回数
	std::unique_ptr<IEnemyAI>	m_pAI;							// AIへのポインタ
};


//*****************************************************************************
// リーダー敵クラス
//*****************************************************************************
class CEnemyLeader : public CEnemy
{
public:
	CEnemyLeader();
	~CEnemyLeader();

	static constexpr float SPEED				= 7.0f;		// 移動スピード
	static constexpr float INVESTIGATE_SPEED	= 14.0f;	// 調査時の移動スピード
	static constexpr float CHASE_SPEED			= 14.0f;	// 追跡時の移動スピード
	static constexpr float DECELERATION_RATE	= 0.85f;	// 減速率
	static constexpr float ACCEL_RATE			= 0.15f;	// 補間率

	// リーダー敵モーションの種類
	typedef enum
	{
		NEUTRAL = 0,			// 待機
		MOVE,					// 移動
		CLOSE_ATTACK_01,		// 近距離攻撃1
		CLOSE_ATTACK_02,		// 近距離攻撃2
		DOUBT,					// 疑い
		SOUND_INVESTIGATE,		// 音源の調査
		TREASURE_INVESTIGATE,	// 埋蔵金の調査
		CAUTION,				// 警戒
		ORDER,					// 命令
		CHASE,					// 追跡
		DISCOVER,				// 発見
		MAX
	}ENEMY_MOTION;

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
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

	// クールダウン設定
	void SetCooldown(float time) { m_Cooldown = time * 60.0f; }

	bool IsCooldown(void) const { return m_Cooldown > 0.0f; }

	CWeaponCollider* GetWeaponCollider(void) { return m_pWeaponCollider.get(); }
	CMotion* GetMotion(void) { return m_pMotion; }
	StateMachine<CEnemyLeader> GetStateMachine(void) { return m_stateMachine; }

private:
	static constexpr int	MAX_PARTS		= 32;			// 最大パーツ数
	static constexpr float	CAPSULE_RADIUS	= 14.0f;		// カプセルコライダーの半径
	static constexpr float	CAPSULE_HEIGHT	= 60.0f;		// カプセルコライダーの高さ
	static constexpr float	COLLIDER_OFFSET = 45.0f;		// コライダーのオフセット位置
	static constexpr float	GRAVITY_RATE	= 5.0f;			// 重力の割合

	CMotion*							m_pMotion;			// モーションへのポインタ
	CShadowS*							m_pShadowS;			// ステンシルシャドウへのポインタ
	CObjectX*							m_pTipModel;		// 武器コライダー用モデル
	CObjectX*							m_pBaseModel;		// 武器コライダー用モデル
	std::unique_ptr<CWeaponCollider>	m_pWeaponCollider;	// 武器の当たり判定へのポインタ
	float								m_Cooldown;			// クールダウン残り時間
	bool								m_bOnGround;		// 接地しているか

	// ステートを管理するクラスのインスタンス
	StateMachine<CEnemyLeader> m_stateMachine;

};

//*****************************************************************************
// サブ敵クラス
//*****************************************************************************
class CEnemySub : public CEnemy
{
public:
	CEnemySub();
	~CEnemySub();

	static constexpr float SPEED				= 5.0f;		// 移動スピード
	static constexpr float INVESTIGATE_SPEED	= 10.0f;	// 調査時の移動スピード
	static constexpr float CHASE_SPEED			= 8.0f;		// 追跡時の移動スピード
	static constexpr float FOLLOW_SPEED			= 15.0f;	// 追従時のスピード
	static constexpr float CHASE_DISTANCE		= 150.0f;	// 追跡状態になる距離
	static constexpr float ACCEL_RATE			= 0.15f;	// 補間率
	static constexpr float DECELERATION_RATE	= 0.9f;		// 減速率

	// サブ敵モーションの種類
	typedef enum
	{
		NEUTRAL = 0,		// 待機
		MOVE,				// 移動
		CHASE,				// 追跡
		INVESTIGATE,		// 調査
		CAUTION,			// 警戒
		FOLLOW,				// リーダー追従
		MAX
	}ENEMY_MOTION;

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
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

	CMotion* GetMotion(void) { return m_pMotion; }
	StateMachine<CEnemySub> GetStateMachine(void) { return m_stateMachine; }

private:
	static constexpr int	MAX_PARTS		= 10;		// 最大パーツ数
	static constexpr float	CAPSULE_RADIUS	= 1.0f;		// カプセルコライダーの半径
	static constexpr float	CAPSULE_HEIGHT	= 65.0f;	// カプセルコライダーの高さ
	static constexpr float	COLLIDER_OFFSET = 45.0f;	// コライダーのオフセット位置
	static constexpr float	GRAVITY_RATE	= 5.0f;		// 重力の割合

	CMotion*	m_pMotion;									// モーションへのポインタ
	bool		m_bOnGround;								// 接地しているか

	// ステートを管理するクラスのインスタンス
	StateMachine<CEnemySub> m_stateMachine;

};

#endif

