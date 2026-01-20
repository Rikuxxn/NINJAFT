//=============================================================================
//
// サブ敵の状態処理 [enemysubState.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _ENEMYSUBSTATE_H_
#define _ENEMYSUBSTATE_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "particle.h"
#include "enemy.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CEnemySub_StandState;
class CEnemySub_MoveState;
class CEnemySub_ChaseState;
class CEnemySub_InvestigateState;
class CEnemySub_CautionState;
class CEnemySub_FollowState;

//*****************************************************************************
// 待機状態
//*****************************************************************************
class CEnemySub_StandState :public StateBase<CEnemySub>
{
public:

	void OnStart(CEnemySub* pEnemy)override
	{
		m_nParticleTimer = 0;

		// 待機モーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemySub::NEUTRAL, 20);
	}

	void OnUpdate(CEnemySub* pEnemy)override
	{
		// 視界内判定をするために、プレイヤーを取得
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		// プレイヤーとの距離を算出
		D3DXVECTOR3 diff = pPlayer->GetPos() - pEnemy->GetPos();
		float distance = D3DXVec3Length(&diff);

		// 減速処理
		pEnemy->ApplyDeceleration();

		// パーティクル発生位置
		D3DXVECTOR3 spawnPos = pEnemy->GetPos();
		spawnPos.y += OFFSET_POS;

		m_nParticleTimer++;

		if (m_nParticleTimer >= PARTICLE_INTERVAL)
		{
			m_nParticleTimer = 0;

			// パーティクル生成
			CParticle::Create<COnibiParticle>(INIT_VEC3, spawnPos, D3DXCOLOR(0.8f, 0.6f, 1.0f, 0.8f), 40, 1);
		}

		// モーション中にプレイヤーが一定範囲近づいたら
		if (distance < CEnemySub::CHASE_DISTANCE)
		{
			// 追跡状態リクエスト
			pEnemy->SetRequestedAction(CEnemy::AI_CHASE);

			// 追跡状態
			m_pMachine->ChangeState<CEnemySub_ChaseState>();
			return;// 即移行
		}

		// 調査状態リクエストされていたら
		if (pEnemy->GetRequestedAction() == CEnemy::AI_SOUND_INVESTIGATE)
		{
			// 調査状態
			m_pMachine->ChangeState<CEnemySub_InvestigateState>();
			return; // すぐに切り替え
		}

		// 追跡状態リクエストされていたら
		if (pEnemy->GetRequestedAction() == CEnemy::AI_CHASE)
		{
			m_pMachine->ChangeState<CEnemySub_ChaseState>();
			return; // すぐに切り替え
		}

		// 待機モーションが終わっていたら
		if (pEnemy->GetMotion()->IsCurrentMotionEnd(CEnemySub::NEUTRAL))
		{
			switch (pEnemy->GetRequestedAction())
			{
			case CEnemy::EEnemyAction::AI_MOVE:// 移動状態
				m_pMachine->ChangeState<CEnemySub_MoveState>();
				break;

			case CEnemy::EEnemyAction::AI_CAUTION:// 警戒状態
				m_pMachine->ChangeState<CEnemySub_CautionState>();
				break;

			case CEnemy::EEnemyAction::AI_FOLLOW:// リーダー追従状態
				m_pMachine->ChangeState<CEnemySub_FollowState>();
				break;

			default:
				break; // そのまま待機
			}
		}
	}

	void OnExit(CEnemySub* /*pEnemy*/)override
	{

	}

private:
	static constexpr int	PARTICLE_INTERVAL	= 6;		// パーティクル生成インターバル
	static constexpr float	OFFSET_POS			= 50.0f;	// オフセット位置

	int m_nParticleTimer;
};

//*****************************************************************************
// 移動状態
//*****************************************************************************
class CEnemySub_MoveState :public StateBase<CEnemySub>
{
public:

	void OnStart(CEnemySub* pEnemy)override
	{
		m_nParticleTimer = 0;

		// 移動モーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemySub::MOVE, 10);
	}

	void OnUpdate(CEnemySub* pEnemy)override
	{
		// 距離を求めるために、プレイヤーを取得
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		// プレイヤーとの距離を算出
		D3DXVECTOR3 diff = pPlayer->GetPos() - pEnemy->GetPos();
		float distance = D3DXVec3Length(&diff);

		// モーション中にプレイヤーが一定範囲近づいたら
		if (distance < CEnemySub::CHASE_DISTANCE)
		{
			// 追跡状態リクエスト
			pEnemy->SetRequestedAction(CEnemy::AI_CHASE);

			// 追跡状態
			m_pMachine->ChangeState<CEnemySub_ChaseState>();
			return;// 即移行
		}

		// リーダーが命令を出したら
		if (pEnemy->IsLeaderAction(CEnemy::AI_ORDER))
		{
			// 音の位置を設定
			pEnemy->OnSoundHeard(pEnemy->GetLastHeardSoundPos());

			// 調査状態
			m_pMachine->ChangeState<CEnemySub_InvestigateState>();
			return;
		}

		// パーティクル発生位置
		D3DXVECTOR3 spawnPos = pEnemy->GetPos();
		spawnPos.y += OFFSET_POS;

		m_nParticleTimer++;

		if (m_nParticleTimer >= PARTICLE_INTERVAL)
		{
			m_nParticleTimer = 0;

			// パーティクル生成
			CParticle::Create<COnibiParticle>(INIT_VEC3, spawnPos, D3DXCOLOR(0.8f, 0.6f, 1.0f, 0.8f), 40, 1);
		}

		// 巡回ポイントに向かう
		D3DXVECTOR3 toTarget = pEnemy->GetPatrolTarget() - pEnemy->GetPos();
		toTarget.y = 0;
		D3DXVec3Normalize(&toTarget, &toTarget);

		// 目標速度計算
		float moveSpeed = CEnemySub::SPEED;
		D3DXVECTOR3 targetMove = pEnemy->GetForward();

		if (targetMove.x != 0.0f || targetMove.z != 0.0f)
		{
			D3DXVec3Normalize(&targetMove, &targetMove);

			targetMove *= moveSpeed;
		}
		else
		{
			targetMove = D3DXVECTOR3(0, 0, 0);
		}

		// 現在速度との補間（イージング）
		D3DXVECTOR3 currentMove = pEnemy->GetMove();

		currentMove.x += (targetMove.x - currentMove.x) * CEnemySub::ACCEL_RATE;
		currentMove.z += (targetMove.z - currentMove.z) * CEnemySub::ACCEL_RATE;

		// 補間後の速度をセット
		pEnemy->SetMove(currentMove);

		// 目標の角度を算出
		float targetYaw = atan2f(-toTarget.x, -toTarget.z);

		// 目的角度を設定（X,Zはそのまま）
		D3DXVECTOR3 rotDest = pEnemy->GetRot();
		rotDest.y = targetYaw;

		// 敵に目的角度を設定
		pEnemy->SetRotDest(rotDest);

		// 補間して回転
		pEnemy->UpdateRotation(0.05f);

		// ----------------------
		// 巡回ポイント到達判定
		// ----------------------
		if (pEnemy->HasReachedTarget())
		{
			if ((rand() % 100) < PROBABILITY)
			{
				// 今到達した巡回ポイントの到達判定がずっと通ってしまうため、次の巡回ポイントを設定しておく
				pEnemy->ChooseNextPatrolPoint();

				// 待機状態
				m_pMachine->ChangeState<CEnemySub_StandState>();
			}
			else
			{
				// 確率で前回立てた音の場所に調査に向かう
				if ((rand() % 100) < PROBABILITY)
				{
					// 音の位置を設定
					pEnemy->OnSoundHeard(pEnemy->GetLastHeardSoundPos());

					// 調査状態
					m_pMachine->ChangeState<CEnemySub_InvestigateState>();
				}
				else
				{
					// 次の巡回ポイントに向かう
					pEnemy->ChooseNextPatrolPoint();
				}
			}
		}
	}

	void OnExit(CEnemySub* /*pEnemy*/)override
	{

	}

private:
	static constexpr int	PARTICLE_INTERVAL	= 6;		// パーティクル生成インターバル
	static constexpr float	OFFSET_POS			= 50.0f;	// オフセット位置
	static constexpr int	PROBABILITY			= 55;		// 確率
	int m_nParticleTimer;

};

//*****************************************************************************
// 追跡状態
//*****************************************************************************
class CEnemySub_ChaseState :public StateBase<CEnemySub>
{
public:

	void OnStart(CEnemySub* pEnemy)override
	{
		m_nParticleTimer = 0;

		// 追跡モーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemySub::CHASE, 10);
	}

	void OnUpdate(CEnemySub* pEnemy)override
	{
		// 追跡状態
		pEnemy->SetRequestedAction(CEnemy::AI_CHASE);

		// パーティクル発生位置
		D3DXVECTOR3 spawnPos = pEnemy->GetPos();
		spawnPos.y += OFFSET_POS;

		m_nParticleTimer++;

		if (m_nParticleTimer >= PARTICLE_INTERVAL)
		{
			m_nParticleTimer = 0;

			// パーティクル生成
			CParticle::Create<COnibiParticle>(INIT_VEC3, spawnPos, D3DXCOLOR(1.0f, 0.2f, 0.4f, 0.6f), 40, 1);
		}

		// プレイヤーの取得
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		// プレイヤーに向かう
		D3DXVECTOR3 dir = pPlayer->GetPos() - pEnemy->GetPos();
		float distance = D3DXVec3Length(&dir);

		// 目標の角度を算出
		float targetYaw = atan2f(-dir.x, -dir.z);

		// 目的角度を設定（X,Zはそのまま）
		D3DXVECTOR3 rotDest = pEnemy->GetRot();
		rotDest.y = targetYaw;

		// 敵に目的角度を設定
		pEnemy->SetRotDest(rotDest);

		// 補間して回転
		pEnemy->UpdateRotation(0.05f);

		// 一定距離離れたら追跡終了
		if (distance > CEnemySub::CHASE_DISTANCE)
		{
			// 最初の巡回ポイントを決めておく
			pEnemy->ReturnToPatrol();

			// 待機状態
			m_pMachine->ChangeState<CEnemySub_StandState>();

			return;
		}

		// 一定距離になったら速度を落とす
		if (distance < DECELERATION_DISTANCE)
		{
			// 減速処理
			pEnemy->ApplyDeceleration();

			return; // 近すぎたら止まる
		}

		D3DXVec3Normalize(&dir, &dir);

		// 目標速度計算
		float moveSpeed = CEnemySub::CHASE_SPEED;
		D3DXVECTOR3 targetMove = dir * moveSpeed;

		if (targetMove.x != 0.0f || targetMove.z != 0.0f)
		{
			D3DXVec3Normalize(&targetMove, &targetMove);

			targetMove *= moveSpeed;
		}
		else
		{
			targetMove = D3DXVECTOR3(0, 0, 0);
		}

		// 現在速度との補間（イージング）
		D3DXVECTOR3 currentMove = pEnemy->GetMove();

		currentMove.x += (targetMove.x - currentMove.x) * CEnemySub::ACCEL_RATE;
		currentMove.z += (targetMove.z - currentMove.z) * CEnemySub::ACCEL_RATE;

		// 補間後の速度をセット
		pEnemy->SetMove(currentMove);

		// 追跡モーションが終わっていたら
		if (pEnemy->GetMotion()->IsCurrentMotionEnd(CEnemySub::CHASE))
		{
			// 待機状態
			m_pMachine->ChangeState<CEnemySub_StandState>();
		}
	}

	void OnExit(CEnemySub* /*pEnemy*/)override
	{

	}

private:
	static constexpr int	PARTICLE_INTERVAL		= 6;		// パーティクル生成インターバル
	static constexpr float	OFFSET_POS				= 50.0f;	// オフセット位置
	static constexpr float	DECELERATION_DISTANCE	= 50.0f;	// 減速開始する距離

	int m_nParticleTimer;
};

//*****************************************************************************
// 調査状態
//*****************************************************************************
class CEnemySub_InvestigateState :public StateBase<CEnemySub>
{
public:

	void OnStart(CEnemySub* pEnemy)override
	{
		m_nParticleTimer = 0;

		// 調査モーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemySub::INVESTIGATE, 10);
	}

	void OnUpdate(CEnemySub* pEnemy)override
	{
		// 距離を求めるために、プレイヤーを取得
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		// プレイヤーとの距離を算出
		D3DXVECTOR3 diff = pPlayer->GetPos() - pEnemy->GetPos();
		float distance = D3DXVec3Length(&diff);

		// モーション中にプレイヤーが一定範囲近づいたら
		if (distance < CHASE_DISTANCE)
		{
			// 追跡状態リクエスト
			pEnemy->SetRequestedAction(CEnemy::AI_CHASE);

			// 追跡状態
			m_pMachine->ChangeState<CEnemySub_ChaseState>();
			return;// 即移行
		}

		// リーダーが命令を出したら
		if (pEnemy->IsLeaderAction(CEnemy::AI_ORDER))
		{
			// 音の位置を設定
			pEnemy->OnSoundHeard(pPlayer->GetPos());
		}

		// 追跡状態リクエストされていたら
		if (pEnemy->GetRequestedAction() == CEnemy::AI_CHASE)
		{
			m_pMachine->ChangeState<CEnemySub_ChaseState>();
			return; // すぐに切り替え
		}

		// パーティクル発生位置
		D3DXVECTOR3 spawnPos = pEnemy->GetPos();
		spawnPos.y += OFFSET_POS;

		m_nParticleTimer++;

		if (m_nParticleTimer >= PARTICLE_INTERVAL)
		{
			m_nParticleTimer = 0;

			// パーティクル生成
			CParticle::Create<COnibiParticle>(INIT_VEC3, spawnPos, D3DXCOLOR(1.0f, 1.0f, 0.6f, 0.6f), 40, 1);
		}

		// 音源のポイントに向かう
		D3DXVECTOR3 toTarget = pEnemy->GetLastHeardSoundPos() - pEnemy->GetPos();
		toTarget.y = 0;
		D3DXVec3Normalize(&toTarget, &toTarget);

		// 目標速度計算
		float moveSpeed = CEnemySub::INVESTIGATE_SPEED;
		D3DXVECTOR3 targetMove = pEnemy->GetForward();

		if (targetMove.x != 0.0f || targetMove.z != 0.0f)
		{
			D3DXVec3Normalize(&targetMove, &targetMove);

			targetMove *= moveSpeed;
		}
		else
		{
			targetMove = D3DXVECTOR3(0, 0, 0);
		}

		// 現在速度との補間（イージング）
		D3DXVECTOR3 currentMove = pEnemy->GetMove();

		currentMove.x += (targetMove.x - currentMove.x) * CEnemySub::ACCEL_RATE;
		currentMove.z += (targetMove.z - currentMove.z) * CEnemySub::ACCEL_RATE;

		// 補間後の速度をセット
		pEnemy->SetMove(currentMove);

		// 目標の角度を算出
		float targetYaw = atan2f(-toTarget.x, -toTarget.z);

		// 目的角度を設定（X,Zはそのまま）
		D3DXVECTOR3 rotDest = pEnemy->GetRot();
		rotDest.y = targetYaw;

		// 敵に目的角度を設定
		pEnemy->SetRotDest(rotDest);

		// 補間して回転
		pEnemy->UpdateRotation(0.05f);

		// 到達したら次の目標に切り替える
		if (pEnemy->HasReachedSoundTarget())
		{
			// 巡回ポイントに到達したら警戒状態
			if (!pEnemy->IsInvestigating())
			{
				m_pMachine->ChangeState<CEnemySub_CautionState>();
			}
		}
	}

	void OnExit(CEnemySub* /*pEnemy*/)override
	{

	}

private:
	static constexpr int	PARTICLE_INTERVAL	= 6;		// パーティクル生成インターバル
	static constexpr float	OFFSET_POS			= 50.0f;	// オフセット位置
	static constexpr float	CHASE_DISTANCE		= 150.0f;	// 追跡開始距離

	int m_nParticleTimer;
};

//*****************************************************************************
// 警戒状態
//*****************************************************************************
class CEnemySub_CautionState :public StateBase<CEnemySub>
{
public:

	void OnStart(CEnemySub* pEnemy)override
	{
		m_nParticleTimer = 0;

		// 警戒モーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemySub::CAUTION, 20);
	}

	void OnUpdate(CEnemySub* pEnemy)override
	{
		// 距離を求めるために、プレイヤーを取得
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		// プレイヤーとの距離を算出
		D3DXVECTOR3 diff = pPlayer->GetPos() - pEnemy->GetPos();
		float distance = D3DXVec3Length(&diff);

		// 減速処理
		pEnemy->ApplyDeceleration();

		// パーティクル発生位置
		D3DXVECTOR3 spawnPos = pEnemy->GetPos();
		spawnPos.y += OFFSET_POS;

		m_nParticleTimer++;

		if (m_nParticleTimer >= PARTICLE_INTERVAL)
		{
			m_nParticleTimer = 0;

			// パーティクル生成
			CParticle::Create<COnibiParticle>(INIT_VEC3, spawnPos, D3DXCOLOR(0.8f, 0.6f, 1.0f, 0.8f), 40, 1);
		}

		// モーション中にプレイヤーが一定範囲近づいたら
		if (distance < CHASE_DISTANCE)
		{
			// 追跡状態リクエスト
			pEnemy->SetRequestedAction(CEnemy::AI_CHASE);

			// 追跡状態
			m_pMachine->ChangeState<CEnemySub_ChaseState>();
			return;// 即移行
		}

		// リーダーが命令を出したら
		if (pEnemy->IsLeaderAction(CEnemy::AI_ORDER))
		{
			// 音の位置を設定
			pEnemy->OnSoundHeard(pPlayer->GetPos());

			// 調査状態
			m_pMachine->ChangeState<CEnemySub_InvestigateState>();
			return;
		}

		switch (pEnemy->GetRequestedAction())
		{
		case CEnemy::EEnemyAction::AI_SOUND_INVESTIGATE:// 音源の調査状態
			m_pMachine->ChangeState<CEnemySub_InvestigateState>();
			break;
		case CEnemy::EEnemyAction::AI_CHASE:// 追跡状態
			m_pMachine->ChangeState<CEnemySub_ChaseState>();
			break;
		case CEnemy::EEnemyAction::AI_MOVE:// 移動状態
			// 警戒モーションが終わっていたら
			if (pEnemy->GetMotion()->IsCurrentMotionEnd(CEnemySub::CAUTION))
			{
				pEnemy->ReturnToPatrol();
				m_pMachine->ChangeState<CEnemySub_MoveState>();
			}
			break;
		}

		// 警戒モーションが終わっていたら
		if (pEnemy->GetMotion()->IsCurrentMotionEnd(CEnemySub::CAUTION))
		{
			// リーダー追従状態
			m_pMachine->ChangeState<CEnemySub_FollowState>();
		}
	}

	void OnExit(CEnemySub* /*pEnemy*/)override
	{

	}

private:
	static constexpr int	PARTICLE_INTERVAL	= 6;		// パーティクル生成
	static constexpr float	OFFSET_POS			= 50.0f;	// オフセット位置
	static constexpr float	CHASE_DISTANCE		= 150.0f;	// 追跡開始距離

	int m_nParticleTimer;
};

//*****************************************************************************
// 追従状態
//*****************************************************************************
class CEnemySub_FollowState :public StateBase<CEnemySub>
{
public:

	void OnStart(CEnemySub* pEnemy)override
	{
		m_nParticleTimer = 0;

		// 追従モーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemySub::FOLLOW, 10);
	}

	void OnUpdate(CEnemySub* pEnemy)override
	{
		switch (pEnemy->GetRequestedAction())
		{
		case CEnemy::EEnemyAction::AI_SOUND_INVESTIGATE:// 調査状態
			m_pMachine->ChangeState<CEnemySub_InvestigateState>();
			break;
		case CEnemy::EEnemyAction::AI_CHASE:// 追跡状態
			m_pMachine->ChangeState<CEnemySub_ChaseState>();
			break;
		case CEnemy::EEnemyAction::AI_MOVE:// 移動状態
			m_pMachine->ChangeState<CEnemySub_MoveState>();
			break;
		}

		// パーティクル発生位置
		D3DXVECTOR3 spawnPos = pEnemy->GetPos();
		spawnPos.y += OFFSET_POS;

		m_nParticleTimer++;

		if (m_nParticleTimer >= PARTICLE_INTERVAL)
		{
			m_nParticleTimer = 0;

			// パーティクル生成
			CParticle::Create<COnibiParticle>(INIT_VEC3, spawnPos, D3DXCOLOR(0.7f, 0.6f, 1.0f, 0.5f), 40, 1);
		}

		CEnemyLeader* pEnemyLeader = CCharacterManager::GetInstance().GetCharacter<CEnemyLeader>();

		if (pEnemyLeader)
		{
			// リーダーに向かう
			D3DXVECTOR3 dir = pEnemyLeader->GetPos() - pEnemy->GetPos();
			float distance = D3DXVec3Length(&dir);

			// 目標の角度を算出
			float targetYaw = atan2f(-dir.x, -dir.z);

			// 目的角度を設定（X,Zはそのまま）
			D3DXVECTOR3 rotDest = pEnemy->GetRot();
			rotDest.y = targetYaw;

			// 敵に目的角度を設定
			pEnemy->SetRotDest(rotDest);

			// 補間して回転
			pEnemy->UpdateRotation(0.05f);

			// 一定距離近づいたら減速する
			if (distance < DECELERATION_DISTANCE)
			{
				// 減速処理
				pEnemy->ApplyDeceleration();

				return; // 近すぎたら止まる
			}

			D3DXVec3Normalize(&dir, &dir);

			// 目標速度計算
			float moveSpeed = CEnemySub::FOLLOW_SPEED;
			D3DXVECTOR3 targetMove = dir * moveSpeed;

			if (targetMove.x != 0.0f || targetMove.z != 0.0f)
			{
				D3DXVec3Normalize(&targetMove, &targetMove);

				targetMove *= moveSpeed;
			}
			else
			{
				targetMove = D3DXVECTOR3(0, 0, 0);
			}

			// 現在速度との補間（イージング）
			D3DXVECTOR3 currentMove = pEnemy->GetMove();

			currentMove.x += (targetMove.x - currentMove.x) * CEnemySub::ACCEL_RATE;
			currentMove.z += (targetMove.z - currentMove.z) * CEnemySub::ACCEL_RATE;

			// 補間後の速度をセット
			pEnemy->SetMove(currentMove);
		}
	}

	void OnExit(CEnemySub* /*pEnemy*/)override
	{

	}

private:
	static constexpr int	PARTICLE_INTERVAL		= 6;		// パーティクル生成
	static constexpr float	OFFSET_POS				= 50.0f;	// オフセット位置
	static constexpr float	DECELERATION_DISTANCE	= 100.0f;	// 減速開始する距離

	int m_nParticleTimer;
};

#endif
