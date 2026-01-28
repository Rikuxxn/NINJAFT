//=============================================================================
//
// プレイヤーの状態処理 [playerState.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _PLAYERSTATE_H_// このマクロ定義がされていなかったら
#define _PLAYERSTATE_H_//2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "meshcylinder.h"
#include "blocklist.h"
#include "blood.h"
#include "specbase.h"
#include "generateMap.h"
#include "waterfield.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CPlayer_StandState;
class CPlayer_MoveState;
class CPlayer_StealthMoveState;
class CPlayer_DamageState;
class CPlayer_StartState;
class CPlayer_InjuryState;
class CPlayer_DigUpState;

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
		CPlayer::InputData input = pPlayer->GatherInput();

		// 入力状態を更新
		pPlayer->UpdateMovementFlags(input.moveDir);

		// プレイヤーHPが少ない
		IsHpFew hpFew;

		// 減速処理
		pPlayer->ApplyDeceleration();

		// 移動入力とステルスボタンが押されていたら
		if (pPlayer->GetIsMoving() && input.stealth)
		{
			// 忍び足移動状態へ移行
			m_pMachine->ChangeState<CPlayer_StealthMoveState>();

			return;
		}
		// 移動入力のみだったら移動ステートに移行
		else if (pPlayer->GetIsMoving() && !input.stealth)
		{
			if (hpFew.IsSatisfiedBy(*pPlayer))
			{
				// 負傷状態へ移行
				m_pMachine->ChangeState<CPlayer_InjuryState>();
			}
			else
			{
				// 移動状態へ移行
				m_pMachine->ChangeState<CPlayer_MoveState>();
			}

			return;
		}

		// --- 埋蔵金ブロックの範囲内か確認 ---
		auto buriedTreasureBlocks = CBlockManager::GetBlocksOfType<CBuriedTreasureBlock>();

		// 埋蔵金が存在しなかったらreturn
		if (!buriedTreasureBlocks.empty())
		{
			for (CBuriedTreasureBlock* treasure : buriedTreasureBlocks)
			{
				// 埋蔵金の方向
				D3DXVECTOR3 toTarget = pPlayer->GetPos() - treasure->GetPos();
				toTarget.y = 0;

				// 距離を求めて正規化する
				float distance = D3DXVec3Length(&toTarget);
				D3DXVec3Normalize(&toTarget, &toTarget);

				// 範囲外の場合は飛ばす
				if (distance < TRIGGER_DISTANCE)
				{
					// 対象に設定
					pPlayer->SetTargetTreasure(treasure);

					// 掘り出し状態へ移行
					m_pMachine->ChangeState<CPlayer_DigUpState>();
				}
			}
		}
	}

	void OnExit(CPlayer* /*pPlayer*/)override
	{

	}

private:
	static constexpr float TRIGGER_DISTANCE = 40.0f;// 判定距離

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
	}

	void OnUpdate(CPlayer* pPlayer)override
	{
		// 入力取得
		CPlayer::InputData input = pPlayer->GatherInput();

		// フラグ更新
		pPlayer->UpdateMovementFlags(input.moveDir);

		// プレイヤーHPが少ない
		IsHpFew hpFew;

		// 満たしていたら
		if (hpFew.IsSatisfiedBy(*pPlayer))
		{
			// 負傷状態
			m_pMachine->ChangeState<CPlayer_InjuryState>();
			return;
		}

		// 埋蔵金の取得数に応じてスピードを遅くする
		int treasureCount = CBuriedTreasureBlock::GetTreasureCount();

		// 水に入ったか
		bool playerInWater = CGenerateMap::GetInstance()->GetWaterField()->IsInWater(pPlayer->GetPos());

		// 入力処理の取得
		CInputKeyboard* pKeyboard = CManager::GetInputKeyboard();	// キーボードの取得
		CInputJoypad* pJoypad = CManager::GetInputJoypad();			// ジョイパッドの取得

		// 埋蔵金取得量に応じたスピード
		float speedRate = 1.0f - treasureCount * DEC_SPEED_RATE;// 5%ずつ低下
		speedRate = std::max(speedRate, MAX_DEC_RATE); // 最大50%

		// ダッシュ入力
		if ((pKeyboard->GetPress(DIK_LSHIFT) || pJoypad->GetPress(CInputJoypad::JOYKEY_RB)) &&
			treasureCount <= DASH_PARTICLE_INTERVAL)
		{
			m_dushTimer++;

			if (m_dushTimer >= DASH_PARTICLE_INTERVAL && !playerInWater)
			{
				m_dushTimer = 0;

				// 生成位置
				D3DXVECTOR3 spawnPos = pPlayer->GetPos();
				spawnPos.y += SPAWN_OFFPOS;

				// 生成方向
				D3DXVECTOR3 dir = pPlayer->GetForward();

				// パーティクル生成
				CParticle::Create<CDushParticle>(INIT_VEC3, spawnPos, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 90, 3);
			}

			// ダッシュ時はスピードレートを上げる
			speedRate = DUSH_SPEED_RATE;
			
			// 埋蔵金を取るたびにレートを落とす
			float NewSpeedRate = speedRate - treasureCount * DEC_NEWSPEED_RATE;// 8%ずつ低下
			NewSpeedRate = std::max(NewSpeedRate, MAX_DEC_RATE); // 最大50%

			speedRate = NewSpeedRate;
		}
		else
		{
			m_dushTimer = 0;

			// モーションスピードを遅くしていく
			pPlayer->GetMotion()->SetMotionSpeedRate(speedRate);
		}

		// 目標速度計算
		float moveSpeed = CPlayer::PLAYER_SPEED * speedRate;

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
		D3DXVECTOR3 currentMove = pPlayer->GetMove();

		currentMove.x += (targetMove.x - currentMove.x) * CPlayer::ACCEL_RATE;
		currentMove.z += (targetMove.z - currentMove.z) * CPlayer::ACCEL_RATE;

		// 補間後の速度をプレイヤーにセット
		pPlayer->SetMove(currentMove);
		
		// プレイヤーの位置取得
		D3DXVECTOR3 pos = pPlayer->GetPos();

		// ステルスかつ移動していたら
		if (input.stealth && pPlayer->GetIsMoving())
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

	void OnExit(CPlayer* pPlayer)override
	{
		// モーションスピードを通常に戻す
		pPlayer->GetMotion()->SetMotionSpeedRate(1.0f);
	}

private:
	static constexpr int	DASH_PARTICLE_INTERVAL	= 5;		// パーティクル発生間隔（フレーム数）
	static constexpr float	SPAWN_OFFPOS			= 20.0f;	// 生成位置オフセット
	static constexpr float	DUSH_SPEED_RATE			= 1.5f;		// ダッシュ時のスピードレート
	static constexpr float	DEC_SPEED_RATE			= 0.05f;	// ダッシュスピードの減少率
	static constexpr float	DEC_NEWSPEED_RATE		= 0.08f;	// 埋蔵金取得時のダッシュスピードの減少率
	static constexpr float	MAX_DEC_RATE			= 0.5f;		// 減少する最大割合(%)

	int m_dushTimer;
};

//*****************************************************************************
// プレイヤーの負傷状態
//*****************************************************************************
class CPlayer_InjuryState :public StateBase<CPlayer>
{
public:

	void OnStart(CPlayer* pPlayer)override
	{
		// 負傷モーション
		pPlayer->GetMotion()->StartBlendMotion(CPlayer::INJURY, 10);
	}

	void OnUpdate(CPlayer* pPlayer)override
	{
		// 入力取得
		CPlayer::InputData input = pPlayer->GatherInput();

		// フラグ更新
		pPlayer->UpdateMovementFlags(input.moveDir);

		// 目標速度計算
		float moveSpeed = CPlayer::INJURY_SPEED /** speedRate*/;

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
		D3DXVECTOR3 currentMove = pPlayer->GetMove();

		currentMove.x += (targetMove.x - currentMove.x) * CPlayer::ACCEL_RATE;
		currentMove.z += (targetMove.z - currentMove.z) * CPlayer::ACCEL_RATE;

		// 補間後の速度をプレイヤーにセット
		pPlayer->SetMove(currentMove);

		// 位置を取得
		D3DXVECTOR3 playerPos = pPlayer->GetPos();

		// プレイヤーの位置取得
		D3DXVECTOR3 pos = pPlayer->GetPos();

		// プレイヤーHPが少ない
		IsHpFew hpFew;

		// 満たしていたら
		if (hpFew.IsSatisfiedBy(*pPlayer))
		{
			m_bloodTimer++;

			if (m_bloodTimer >= BLOOD_INTERVAL)
			{
				m_bloodTimer = 0;

				// 地面に埋もれないように少し上に上げる
				pos.y += OFFSET_HEIGHT;

				// 血痕のサイズをランダムにする
				float size = (rand()% 15) + 5.0f;

				// 血痕の生成
				CBlood::Create(pos, D3DXVECTOR3(90.0f, 0.0f, 0.0f), D3DXCOLOR(0.5f, 0.5f, 0.5f, 1.0f), size, size);
			}
		}

		// ステルスかつ移動していたら
		if (input.stealth && pPlayer->GetIsMoving())
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
	static constexpr int	BLOOD_INTERVAL	= 50;	// 血痕生成間隔（フレーム数）
	static constexpr float	OFFSET_HEIGHT	= 2.0f;	// 血痕の高さのオフセット

	int m_bloodTimer;		// 血痕生成タイマー
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
		CPlayer::InputData input = pPlayer->GatherInput();

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
		D3DXVECTOR3 currentMove = pPlayer->GetMove();

		currentMove.x += (targetMove.x - currentMove.x) * CPlayer::ACCEL_RATE;
		currentMove.z += (targetMove.z - currentMove.z) * CPlayer::ACCEL_RATE;

		// 補間後の速度をプレイヤーにセット
		pPlayer->SetMove(currentMove);

		// 移動入力があれば移動ステートに移行
		if (pPlayer->GetIsMoving() && !input.stealth)
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
		pPlayer->SetDamagePhysics(true);

		// 念のため接地解除
		pPlayer->SetOnGround(false);

		// 音の取得
		CSound* pSound = CManager::GetSound();

		// ジョイパッドの取得
		CInputJoypad* pJoypad = CManager::GetInputJoypad();

		if (pJoypad)
		{
			// 振動させる
			pJoypad->SetVibration(20000, 20000, 40);
		}

		// ダメージSEの再生
		if (pSound)
		{
			pSound->Play(CSound::SOUND_LABEL_DAMAGE);
		}

		// ダメージモーション
		pPlayer->GetMotion()->StartBlendMotion(CPlayer::DAMAGE, 10);
		
		// リーダー敵の取得
		CEnemy* pEnemy = CCharacterManager::GetInstance().GetCharacter<CEnemy>();

		if (!pEnemy)
		{
			return;
		}

		// 最初の勢い（後方への初速）
		D3DXVECTOR3 damageDir = pEnemy->GetForward();
		D3DXVec3Normalize(&damageDir, &damageDir);

		D3DXVECTOR3 move = damageDir * FORWARD_POWER;

		pPlayer->SetMove(move);

		// 上方向初速
		m_verticalVelocity = FIRST_UP_VELOCITY;

		// 物理に反映
		btVector3 velocity = pPlayer->GetRigidBody()->getLinearVelocity();
		velocity.setX(move.x);
		velocity.setY(m_verticalVelocity);
		velocity.setZ(move.z);
		pPlayer->GetRigidBody()->setLinearVelocity(velocity);

		//// 生成位置
		//D3DXVECTOR3 spawnPos = pPlayer->GetPos();
		//spawnPos.y += 70.0f;

		//// 生成方向
		//D3DXVECTOR3 dir = pPlayer->GetForward();

		//for (int n = 0; n < 6; n++)
		//{
		//	// 血しぶきパーティクル生成
		//	CParticle::Create<CBloodSplatter>(-dir, spawnPos, D3DXCOLOR(0.8f, 0.0f, 0.0f, 0.8f), 900, 3);
		//}
	}

	void OnUpdate(CPlayer* pPlayer)override
	{
		// モーションの進行度を取得
		float motionRate = pPlayer->GetMotion()->GetMotionRate(); // 0.0～1.0
		D3DXVECTOR3 move = pPlayer->GetMove();

		// モーション前半だけ前方移動を維持
		if (motionRate < MOTIONRATE_THRESHOLD)
		{
			D3DXVECTOR3 forwardDir = -pPlayer->GetForward();
			D3DXVec3Normalize(&forwardDir, &forwardDir);

			move = forwardDir * SLIDE_POWER;

			// 着地したらfalseに戻す
			if (pPlayer->GetOnGround())
			{
				pPlayer->SetDamagePhysics(false);
			}
		}
		else
		{
			// 後半は減速
			move *= JUMP_DECELERATION_RATE;
			if (fabsf(move.x) < 0.01f) move.x = 0;
			if (fabsf(move.z) < 0.01f) move.z = 0;
		}

		// ===== 重力処理 =====
		m_verticalVelocity -= GRAVITY; // 重力加速度
		if (m_verticalVelocity < MAX_DOWN_SPEED)
		{
			m_verticalVelocity = MAX_DOWN_SPEED; // 最大落下速度を制限
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

	void OnExit(CPlayer* pPlayer)override
	{
		pPlayer->SetDamagePhysics(false);
		m_verticalVelocity = 0.0f;
	}

private:
	static constexpr float FIRST_UP_VELOCITY		= 50.0f;	// 最初の上方向の力
	static constexpr float SLIDE_POWER				= 15.0f;	// スライドパワー
	static constexpr float GRAVITY					= 3.0f;		// 重力加速度
	static constexpr float JUMP_DECELERATION_RATE	= 0.88f;	// ジャンプ時の減速率
	static constexpr float FORWARD_POWER			= 120.0f;	// 初速パワー
	static constexpr float MAX_DOWN_SPEED			= -30.0f;	// 最大落下速度
	static constexpr float MOTIONRATE_THRESHOLD		= 0.4f;		// モーションレートの閾値

	float m_verticalVelocity; // 上下方向速度
};

//*****************************************************************************
// プレイヤーのスタート状態
//*****************************************************************************
class CPlayer_StartState :public StateBase<CPlayer>
{
public:

	void OnStart(CPlayer* pPlayer)override
	{
		// スタートモーション
		pPlayer->GetMotion()->StartBlendMotion(CPlayer::START, 10);
	}

	void OnUpdate(CPlayer* pPlayer)override
	{
		// 減速処理
		pPlayer->ApplyDeceleration();

		if (pPlayer->GetMotion()->IsCurrentMotionEnd(CPlayer::START))
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
// プレイヤーの掘り出し状態
//*****************************************************************************
class CPlayer_DigUpState :public StateBase<CPlayer>
{
public:

	void OnStart(CPlayer* pPlayer)override
	{
		// 掘り出しモーション
		pPlayer->GetMotion()->StartBlendMotion(CPlayer::DIG_UP, 10);
	}

	void OnUpdate(CPlayer* pPlayer)override
	{
		// 減速処理
		pPlayer->ApplyDeceleration();

		// 入力を取得
		CPlayer::InputData input = pPlayer->GatherInput();

		// 入力の更新
		pPlayer->UpdateMovementFlags(input.moveDir);

		// プレイヤーHPが少ない
		IsHpFew hpFew;

		// 移動入力とステルスボタンが押されていたら
		if (pPlayer->GetIsMoving() && input.stealth)
		{
			// 忍び足移動状態へ移行
			m_pMachine->ChangeState<CPlayer_StealthMoveState>();
			return;
		}
		// 移動入力のみだったら移動ステートに移行
		else if (pPlayer->GetIsMoving())
		{
			if (hpFew.IsSatisfiedBy(*pPlayer))
			{
				// 負傷状態へ移行
				m_pMachine->ChangeState<CPlayer_InjuryState>();
			}
			else
			{
				// 移動状態へ移行
				m_pMachine->ChangeState<CPlayer_MoveState>();
			}

			return;
		}

		// 対象の埋蔵金ブロックを取得
		auto* pTreasure = pPlayer->GetTargetTreasure();

		// 対象の埋蔵金ブロックを取得したか確認
		if (!pTreasure || pTreasure->IsFinished())
		{
			// 待機状態へ移行
			m_pMachine->ChangeState<CPlayer_StandState>();
			return;
		}
	}

	void OnExit(CPlayer* /*pPlayer*/)override
	{

	}

private:
	static constexpr float TRIGGER_DISTANCE = 40.0f;// 判定距離

};

#endif