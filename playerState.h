//=============================================================================
//
// プレイヤーの状態処理 [playerState.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _PLAYERSTATE_H_// このマクロ定義がされていなかったら
#define _PLAYERSTATE_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "meshcylinder.h"
#include "blocklist.h"

// 前方宣言
class CPlayer_StandState;
class CPlayer_MoveState;
class CPlayer_StealthMoveState;
class CPlayer_DamageState;

//*****************************************************************************
// プレイヤーの待機状態
//*****************************************************************************
class CPlayer_StandState :public StateBase<CPlayer>
{
public:

	void OnStart(CPlayer* pPlayer)override
	{
		// 待機モーション
		pPlayer->GetMotion()->StartBlendMotion(CPlayer::NEUTRAL, 10);
	}

	void OnUpdate(CPlayer* pPlayer)override
	{
		// 入力を取得
		InputData input = pPlayer->GatherInput();

		// 移動入力とステルスボタンが押されていたら
		if ((input.moveDir.x != 0.0f || input.moveDir.z != 0.0f) && input.stealth)
		{
			// 忍び足移動状態へ移行
			m_pMachine->ChangeState<CPlayer_StealthMoveState>();
		}
		// 移動入力のみだったら移動ステートに移行
		else if((input.moveDir.x != 0.0f || input.moveDir.z != 0.0f) && !input.stealth)
		{
			// 移動状態へ移行
			m_pMachine->ChangeState<CPlayer_MoveState>();
		}

		D3DXVECTOR3 move = pPlayer->GetMove();

		move *= 0.82f; // 減速率
		if (fabsf(move.x) < 0.01f) move.x = 0;
		if (fabsf(move.z) < 0.01f) move.z = 0;

		// 移動量を設定
		pPlayer->SetMove(move);
	}

	void OnExit(CPlayer* /*pPlayer*/)override
	{

	}

private:

};

//*****************************************************************************
// プレイヤーの移動状態
//*****************************************************************************
class CPlayer_MoveState :public StateBase<CPlayer>
{
public:

	void OnStart(CPlayer* pPlayer)override
	{
		// 移動モーション
		pPlayer->GetMotion()->StartBlendMotion(CPlayer::MOVE, 10);

		m_particleTimer = 0;
	}

	void OnUpdate(CPlayer* pPlayer)override
	{
		// 入力取得
		InputData input = pPlayer->GatherInput();

		// フラグ更新
		pPlayer->UpdateMovementFlags(input.moveDir);

		//// 埋蔵金の取得数に応じてスピードを遅くする
		//int treasureCount = CBuriedTreasureBlock::GetTreasureCount();

		// 目標速度計算
		float moveSpeed = CPlayer::PLAYER_SPEED/* * (treasureCount * 0.9f)*/;

		D3DXVECTOR3 targetMove = input.moveDir;

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
		const float accelRate = 0.15f;
		D3DXVECTOR3 currentMove = pPlayer->GetMove();

		currentMove.x += (targetMove.x - currentMove.x) * accelRate;
		currentMove.z += (targetMove.z - currentMove.z) * accelRate;

		// 補間後の速度をプレイヤーにセット
		pPlayer->SetMove(currentMove);

		CModelEffect* pModelEffect = nullptr;

		if (pPlayer->GetIsMoving() && pPlayer->GetOnGround())
		{
			m_particleTimer++;

			if (m_particleTimer >= DASH_PARTICLE_INTERVAL)
			{
				m_particleTimer = 0;

				// ランダムな角度で横に広がる
				float angle = ((rand() % 360) / 180.0f) * D3DX_PI;
				float speed = (rand() % 150) / 300.0f + 0.2f;

				// 移動量
				D3DXVECTOR3 move;
				move.x = cosf(angle) * speed;
				move.z = sinf(angle) * speed;
				move.y = (rand() % 80) / 50.0f + 0.05f; // 少しだけ上方向

				// 向き
				D3DXVECTOR3 rot;
				rot.x = ((rand() % 360) / 180.0f) * D3DX_PI;
				rot.y = ((rand() % 360) / 180.0f) * D3DX_PI;
				rot.z = ((rand() % 360) / 180.0f) * D3DX_PI;

				// モデルエフェクトの生成
				pModelEffect = CModelEffect::Create("data/MODELS/effectModel_step.x", pPlayer->GetPos(), rot,
					move, D3DXVECTOR3(0.3f, 0.3f, 0.3f), 180, 0.01f, 0.008f);
			}
		}
		else
		{
			m_particleTimer = 0; // 停止時はリセット
		}

		if (input.stealth && (input.moveDir.x != 0.0f || input.moveDir.z != 0.0f))
		{
			// 忍び足移動状態へ移行
			m_pMachine->ChangeState<CPlayer_StealthMoveState>();
			return;
		}

		// 移動していなければ待機ステートに戻す
		if (!pPlayer->GetIsMoving())
		{
			// 待機状態
			m_pMachine->ChangeState<CPlayer_StandState>();
		}
	}

	void OnExit(CPlayer* /*pPlayer*/)override
	{

	}

private:
	static constexpr int DASH_PARTICLE_INTERVAL = 10; // パーティクル発生間隔（フレーム数）

	int m_particleTimer;	// パーティクル生成タイマー
};

//*****************************************************************************
// プレイヤーの忍び足移動状態
//*****************************************************************************
class CPlayer_StealthMoveState :public StateBase<CPlayer>
{
public:

	void OnStart(CPlayer* pPlayer)override
	{
		// 忍び足移動モーション
		pPlayer->GetMotion()->StartBlendMotion(CPlayer::STEALTH_MOVE, 10);
	}

	void OnUpdate(CPlayer* pPlayer)override
	{
		// 入力取得
		InputData input = pPlayer->GatherInput();

		// フラグ更新
		pPlayer->UpdateMovementFlags(input.moveDir);

		// 目標速度計算
		float moveSpeed = CPlayer::STEALTH_SPEED;

		D3DXVECTOR3 targetMove = input.moveDir;

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
		const float accelRate = 0.15f;
		D3DXVECTOR3 currentMove = pPlayer->GetMove();

		currentMove.x += (targetMove.x - currentMove.x) * accelRate;
		currentMove.z += (targetMove.z - currentMove.z) * accelRate;

		// 補間後の速度をプレイヤーにセット
		pPlayer->SetMove(currentMove);

		// 移動入力があれば移動ステートに移行
		if ((input.moveDir.x != 0.0f || input.moveDir.z != 0.0f) && !input.stealth)
		{
			// 移動状態へ移行
			m_pMachine->ChangeState<CPlayer_MoveState>();
			return;
		}

		// 移動していなければ待機ステートに戻す
		if(!pPlayer->GetIsMoving())
		{
			// 待機状態
			m_pMachine->ChangeState<CPlayer_StandState>();
		}
	}

	void OnExit(CPlayer* /*pPlayer*/)override
	{


	}

private:

};

//*****************************************************************************
// プレイヤーのダメージ状態
//*****************************************************************************
class CPlayer_DamageState :public StateBase<CPlayer>
{
public:

	void OnStart(CPlayer* pPlayer)override
	{
		// ダメージモーション
		pPlayer->GetMotion()->StartBlendMotion(CPlayer::DAMAGE, 10);

		// 最初の勢い（後方への初速）
		D3DXVECTOR3 damageDir = CGame::GetEnemy()->GetForward();
		D3DXVec3Normalize(&damageDir, &damageDir);

		float forwardPower = 120.0f; // 初速パワー
		D3DXVECTOR3 move = damageDir * forwardPower;

		pPlayer->SetMove(move);

		// 上方向初速
		m_verticalVelocity = 50.0f;

		// 物理に反映
		btVector3 velocity = pPlayer->GetRigidBody()->getLinearVelocity();
		velocity.setX(move.x);
		velocity.setY(m_verticalVelocity);
		velocity.setZ(move.z);
		pPlayer->GetRigidBody()->setLinearVelocity(velocity);
	}

	void OnUpdate(CPlayer* pPlayer)override
	{
		// モーションの進行度を取得
		float motionRate = pPlayer->GetMotion()->GetMotionRate(); // 0.0～1.0
		D3DXVECTOR3 move = pPlayer->GetMove();

		// モーション前半だけ前方移動を維持
		if (motionRate < 0.4f)
		{
			D3DXVECTOR3 forwardDir = -pPlayer->GetForward();
			D3DXVec3Normalize(&forwardDir, &forwardDir);

			float forwardPower = 15.0f; // 滑る速度
			move = forwardDir * forwardPower;
		}
		else
		{
			// 後半は減速
			move *= 0.88f;
			if (fabsf(move.x) < 0.01f) move.x = 0;
			if (fabsf(move.z) < 0.01f) move.z = 0;
		}

		// ===== 重力処理 =====
		m_verticalVelocity -= 3.0f; // 重力加速度
		if (m_verticalVelocity < -30.0f)
		{
			m_verticalVelocity = -30.0f; // 最大落下速度を制限
		}

		// 移動量を設定
		pPlayer->SetMove(move);

		// リジッドボディに反映
		btVector3 velocity = pPlayer->GetRigidBody()->getLinearVelocity();
		velocity.setX(move.x);
		velocity.setY(m_verticalVelocity);
		velocity.setZ(move.z);
		pPlayer->GetRigidBody()->setLinearVelocity(velocity);

		if (pPlayer->GetMotion()->IsCurrentMotionEnd(CPlayer::DAMAGE))
		{
			// 死んだら
			if (pPlayer->IsDead())
			{
				pPlayer->SetIsDead(true);
			}
			else
			{
				// 待機状態
				m_pMachine->ChangeState<CPlayer_StandState>();
			}
		}
	}

	void OnExit(CPlayer* /*pPlayer*/)override
	{
		m_verticalVelocity = 0.0f;
	}

private:
	float m_verticalVelocity; // 上下方向速度
};

#endif