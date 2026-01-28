//=============================================================================
//
// リーダー敵の状態処理 [enemyleaderState.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _ENEMYLEADERSTATE_H_
#define _ENEMYLEADERSTATE_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "particle.h"
#include "enemy.h"
#include "SEpopupeffect.h"
#include "sound.h"
#include "manager.h"
#include "motion.h"
#include "specbase.h"
#include "meshOrbit.h"

//*****************************************************************************
// 前方宣言
//*****************************************************************************
class CEnemyLeader_StandState;
class CEnemyLeader_MoveState;
class CEnemyLeader_CloseAttackState1;
class CEnemyLeader_CloseAttackState2;
class CEnemyLeader_DoubtState;
class CEnemyLeader_SoundInvestigateState;
class CEnemyLeader_TreasureInvestigateState;
class CEnemyLeader_CautionState;
class CEnemyLeader_OrderState;
class CEnemyLeader_ChaseState;
class CEnemyLeader_DiscoverState;

//*****************************************************************************
// リーダー敵の待機状態
//*****************************************************************************
class CEnemyLeader_StandState :public StateBase<CEnemyLeader>
{
public:

	void OnStart(CEnemyLeader* pEnemy)override
	{
		// 待機モーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemyLeader::NEUTRAL, 20);
	}

	void OnUpdate(CEnemyLeader* pEnemy)override
	{
		// 視界内判定のするため、プレイヤーを取得する
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		// 減速処理
		pEnemy->ApplyDeceleration();

		// サブ敵が追跡状態だったら
		if (pEnemy->IsSubAction(CEnemy::AI_CHASE) && !pEnemy->IsCooldown())
		{
			// 追跡状態
			m_pMachine->ChangeState<CEnemyLeader_ChaseState>();
			return;
		}

		// 視界に入ったら
		if (pPlayer && pEnemy->IsPlayerInSight(pPlayer) && !pEnemy->IsCooldown())
		{
			// 発見状態
			m_pMachine->ChangeState<CEnemyLeader_DiscoverState>();
			return;
		}

		// AIリクエストに応じて状態を変更
		switch (pEnemy->GetRequestedAction())
		{
		case CEnemy::EEnemyAction::AI_SOUND_INVESTIGATE:// 音源の調査状態
			m_pMachine->ChangeState<CEnemyLeader_SoundInvestigateState>();
			break;
		case CEnemy::EEnemyAction::AI_ORDER:// 命令状態
			m_pMachine->ChangeState<CEnemyLeader_OrderState>();
			break;
		case CEnemy::EEnemyAction::AI_DOUBT:// 疑い状態
			m_pMachine->ChangeState<CEnemyLeader_DoubtState>();
			break;
		default:
			break; // そのまま待機
		}

		// 待機モーションが終わっていたら
		if (pEnemy->GetMotion()->IsCurrentMotionEnd(CEnemyLeader::NEUTRAL))
		{
			// 操作フラグがfalse または クールダウンが設定されているときは待機状態にする
			if (!pEnemy->GetControlFlag() || pEnemy->IsCooldown())
			{
				// 待機状態
				m_pMachine->ChangeState<CEnemyLeader_StandState>();
				return;
			}

			// 移動状態
			m_pMachine->ChangeState<CEnemyLeader_MoveState>();
		}
	}

	void OnExit(CEnemyLeader* /*pEnemy*/)override
	{

	}

private:
};

//*****************************************************************************
// リーダー敵の移動状態
//*****************************************************************************
class CEnemyLeader_MoveState :public StateBase<CEnemyLeader>
{
public:

	void OnStart(CEnemyLeader* pEnemy)override
	{

		// 移動モーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemyLeader::MOVE, 10);
	}

	void OnUpdate(CEnemyLeader* pEnemy)override
	{
		// サブ敵が追跡状態だったら
		if (pEnemy->IsSubAction(CEnemy::AI_CHASE))
		{
			// 疑い状態
			m_pMachine->ChangeState<CEnemyLeader_DoubtState>();
			return;
		}

		switch (pEnemy->GetRequestedAction())
		{
		case CEnemyLeader::AI_DISCOVER:
			// 発見状態
			m_pMachine->ChangeState<CEnemyLeader_DiscoverState>();
			break;
		case CEnemyLeader::AI_ORDER:
			// 命令状態
			m_pMachine->ChangeState<CEnemyLeader_OrderState>();
			break;
		case CEnemyLeader::AI_CHASE:
			// 追跡状態
			m_pMachine->ChangeState<CEnemyLeader_ChaseState>();
			break;
		case CEnemyLeader::AI_DOUBT:
			// 疑い状態
			m_pMachine->ChangeState<CEnemyLeader_DoubtState>();
			break;
		}

		// 巡回ポイントに向かう
		D3DXVECTOR3 toTarget = pEnemy->GetPatrolTarget() - pEnemy->GetPos();
		toTarget.y = 0;
		D3DXVec3Normalize(&toTarget, &toTarget);

		// モーションスピードを早くする
		pEnemy->GetMotion()->SetMotionSpeedRate(MOTION_SPEED_RATE);

		// 目標速度計算
		float moveSpeed = CEnemyLeader::SPEED;
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

		currentMove.x += (targetMove.x - currentMove.x) * CEnemyLeader::ACCEL_RATE;
		currentMove.z += (targetMove.z - currentMove.z) * CEnemyLeader::ACCEL_RATE;

		// 補間後の速度をプレイヤーにセット
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
				m_pMachine->ChangeState<CEnemyLeader_StandState>();
			}
			else
			{
				if ((rand() % 100) < PROBABILITY)
				{
					// 埋蔵金の調査状態
					m_pMachine->ChangeState<CEnemyLeader_TreasureInvestigateState>();
				}
				else
				{
					// 次の巡回ポイントに向かう
					pEnemy->ChooseNextPatrolPoint();
				}
			}
		}
	}

	void OnExit(CEnemyLeader* pEnemy)override
	{
		// モーションスピードを通常にする
		pEnemy->GetMotion()->SetMotionSpeedRate(1.0f);
	}

private:
	static constexpr float	MOTION_SPEED_RATE	= 1.5f;	// モーションのスピードレート
	static constexpr int	PROBABILITY			= 55;	// 確率
};

//*****************************************************************************
// リーダー敵の近距離攻撃状態1
//*****************************************************************************
class CEnemyLeader_CloseAttackState1 :public StateBase<CEnemyLeader>
{
public:

	void OnStart(CEnemyLeader* pEnemy)override
	{
		// ヒットフラグをリセット
		pEnemy->GetWeaponCollider()->ResetHit();

		// 近距離攻撃モーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemyLeader::CLOSE_ATTACK_01, 10);

		// 音の取得
		CSound* pSound = CManager::GetSound();

		// 斬撃SE1の再生
		if (pSound)
		{
			pSound->Play(CSound::SOUND_LABEL_SLASH_1);
		}

		// プレイヤー取得
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		// プレイヤーへの方向ベクトル
		D3DXVECTOR3 toPlayer = pPlayer->GetPos() - pEnemy->GetPos();

		if (pPlayer)
		{
			toPlayer.y = 0.0f; // 水平方向のみ
			D3DXVec3Normalize(&toPlayer, &toPlayer);

			// 目標の角度を算出
			float targetYaw = atan2f(-toPlayer.x, -toPlayer.z);

			// 目的角度を設定（X,Zはそのまま）
			D3DXVECTOR3 rotDest = pEnemy->GetRot();
			rotDest.y = targetYaw;

			// 敵に目的角度を設定
			pEnemy->SetRotDest(rotDest);

			// 補間して回転
			pEnemy->UpdateRotation(0.5f);
		}

		// 向いている方向にスライドさせるため、プレイヤー方向ベクトルを取得
		D3DXVECTOR3 dir = toPlayer;

		// 正規化
		D3DXVec3Normalize(&dir, &dir);

		D3DXVECTOR3 move = dir * SLIDE_POWER;

		// 現在の移動量に上書き
		pEnemy->SetMove(move);

		// 生成開始位置1を決める
		D3DXVECTOR3 startPos1 = pEnemy->GetWeaponCollider()->GetCurrentTipPos();// 剣の先

		// 生成開始位置2を決める
		D3DXVECTOR3 startPos2 = pEnemy->GetWeaponCollider()->GetCurrentBasePos();// 剣の根元

		// メッシュの軌跡の生成
		m_pOrbit = CMeshOrbit::Create(startPos1, startPos2, D3DXCOLOR(0.7f, 0.3f, 1.0f, 0.8f), 50);
	}

	void OnUpdate(CEnemyLeader* pEnemy)override
	{
		// 軌跡の位置更新
		if (m_pOrbit)
		{
			// 生成開始位置1を決める
			D3DXVECTOR3 startPos1 = pEnemy->GetWeaponCollider()->GetCurrentTipPos();// 剣の先

			// 生成開始位置2を決める
			D3DXVECTOR3 startPos2 = pEnemy->GetWeaponCollider()->GetCurrentBasePos();// 剣の根元

			m_pOrbit->SetStartPos(startPos1, startPos2);
		}

		// モーションの進行度を取得
		float motionRate = pEnemy->GetMotion()->GetMotionRate(); // 0.0～1.0
		D3DXVECTOR3 move = pEnemy->GetMove();

		// モーション前半だけ前方移動を維持
		if (motionRate < MOTIONRATE_THRESHOLD)
		{
			D3DXVECTOR3 forwardDir = pEnemy->GetForward();
			D3DXVec3Normalize(&forwardDir, &forwardDir);

			move = forwardDir * FORWARD_POWER;
		}
		else
		{
			move *= CEnemyLeader::DECELERATION_RATE; // 減速率
			if (fabsf(move.x) < 0.01f) move.x = 0;
			if (fabsf(move.z) < 0.01f) move.z = 0;
		}

		// 移動量を設定
		pEnemy->SetMove(move);

		if (pEnemy->GetWeapon() && pEnemy->GetWeaponCollider())
		{
			// 攻撃中だけ有効化
			if (pEnemy->GetMotion()->EventMotionRange(CEnemyLeader::CLOSE_ATTACK_01, 2, 4, 0, 15))
			{
				pEnemy->GetWeaponCollider()->SetActive(true);
				pEnemy->GetWeaponCollider()->ResetPrevPos();
			}
			else
			{
				pEnemy->GetWeaponCollider()->SetActive(false);
			}

			// プレイヤーの取得
			CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

			if (pPlayer)
			{
				// プレイヤーに当たったか判定する
				pEnemy->GetWeaponCollider()->CheckHit(pPlayer, 2.5f, 25.0f);
			}
		}

		if (pEnemy->GetMotion()->IsCurrentMotionEnd(CEnemyLeader::CLOSE_ATTACK_01))
		{// 近距離攻撃モーションが終わっていたら

			// 近距離攻撃状態2
			m_pMachine->ChangeState<CEnemyLeader_CloseAttackState2>();

			return;
		}
	}

	void OnExit(CEnemyLeader* /*pEnemy*/)override
	{

	}

private:
	static constexpr float SLIDE_POWER			= 45.0f;	// スライドパワー
	static constexpr float MOTIONRATE_THRESHOLD = 0.2f;		// モーションレートの閾値
	static constexpr float FORWARD_POWER		= 15.0f;	// 滑る速度

	CMeshOrbit* m_pOrbit = nullptr;

};

//*****************************************************************************
// リーダー敵の近距離攻撃状態2
//*****************************************************************************
class CEnemyLeader_CloseAttackState2 :public StateBase<CEnemyLeader>
{
public:

	void OnStart(CEnemyLeader* pEnemy)override
	{
		// ヒットフラグをリセット
		pEnemy->GetWeaponCollider()->ResetHit();

		// 近距離攻撃モーション2
		pEnemy->GetMotion()->StartBlendMotion(CEnemyLeader::CLOSE_ATTACK_02, 10);

		// 音の取得
		CSound* pSound = CManager::GetSound();

		// 斬撃SE2の再生
		if (pSound)
		{
			pSound->Play(CSound::SOUND_LABEL_SLASH_2);
		}

		// プレイヤー取得
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		if (!pPlayer)
		{
			return;
		}

		// プレイヤーへの方向ベクトル
		D3DXVECTOR3 toPlayer = pPlayer->GetPos() - pEnemy->GetPos();

		// 向いている方向にスライドさせるため、プレイヤー方向ベクトルを取得
		D3DXVECTOR3 dir = toPlayer;

		// 正規化
		D3DXVec3Normalize(&dir, &dir);

		D3DXVECTOR3 move = dir * SLIDE_POWER;

		// 現在の移動量に上書き
		pEnemy->SetMove(move);

		// 生成開始位置1を決める
		D3DXVECTOR3 startPos1 = pEnemy->GetWeaponCollider()->GetCurrentTipPos();// 剣の先

		// 生成開始位置2を決める
		D3DXVECTOR3 startPos2 = pEnemy->GetWeaponCollider()->GetCurrentBasePos();// 剣の根元

		// メッシュの軌跡の生成
		m_pOrbit = CMeshOrbit::Create(startPos1, startPos2, D3DXCOLOR(0.7f, 0.3f, 1.0f, 0.8f), 50);
	}

	void OnUpdate(CEnemyLeader* pEnemy)override
	{
		// 軌跡の位置更新
		if (m_pOrbit)
		{
			// 生成開始位置1を決める
			D3DXVECTOR3 startPos1 = pEnemy->GetWeaponCollider()->GetCurrentTipPos();// 剣の先

			// 生成開始位置2を決める
			D3DXVECTOR3 startPos2 = pEnemy->GetWeaponCollider()->GetCurrentBasePos();// 剣の根元

			m_pOrbit->SetStartPos(startPos1, startPos2);
		}

		// モーションの進行度を取得
		float motionRate = pEnemy->GetMotion()->GetMotionRate(); // 0.0～1.0
		D3DXVECTOR3 move = pEnemy->GetMove();

		// モーション前半だけ前方移動を維持
		if (motionRate < MOTIONRATE_THRESHOLD)
		{
			D3DXVECTOR3 forwardDir = pEnemy->GetForward();
			D3DXVec3Normalize(&forwardDir, &forwardDir);

			move = forwardDir * FORWARD_POWER;
		}
		else
		{
			move *= CEnemyLeader::DECELERATION_RATE; // 減速率
			if (fabsf(move.x) < 0.01f) move.x = 0;
			if (fabsf(move.z) < 0.01f) move.z = 0;
		}

		// 移動量を設定
		pEnemy->SetMove(move);

		if (pEnemy->GetWeapon() && pEnemy->GetWeaponCollider())
		{
			// 攻撃中だけ有効化
			if (pEnemy->GetMotion()->EventMotionRange(CEnemyLeader::CLOSE_ATTACK_02, 0, 2, 0, 15))
			{
				pEnemy->GetWeaponCollider()->SetActive(true);
				pEnemy->GetWeaponCollider()->ResetPrevPos();
			}
			else
			{
				pEnemy->GetWeaponCollider()->SetActive(false);
			}

			// プレイヤーの取得
			CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

			if (pPlayer)
			{
				// プレイヤーに当たったか判定する
				pEnemy->GetWeaponCollider()->CheckHit(pPlayer, 2.5f, 30.0f);
			}
		}

		if (pEnemy->GetMotion()->IsCurrentMotionEnd(CEnemyLeader::CLOSE_ATTACK_02))
		{// 近距離攻撃モーションが終わっていたら
			// 待機状態
			m_pMachine->ChangeState<CEnemyLeader_StandState>();
		}
	}

	void OnExit(CEnemyLeader* pEnemy)override
	{
		// クールダウンの設定
		pEnemy->SetCooldown(3.0f);
	}

private:
	static constexpr float SLIDE_POWER			= 20.0f;	// スライドパワー
	static constexpr float FORWARD_POWER		= 10.0f;	// 滑る速度
	static constexpr float MOTIONRATE_THRESHOLD = 0.2f;		// モーションレートの閾値

	CMeshOrbit* m_pOrbit = nullptr;
};

//*****************************************************************************
// リーダー敵の疑い状態
//*****************************************************************************
class CEnemyLeader_DoubtState :public StateBase<CEnemyLeader>
{
public:

	void OnStart(CEnemyLeader* pEnemy)override
	{
		m_timer = 0;

		// 疑いモーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemyLeader::DOUBT, 10);

		// プレイヤー取得
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		// プレイヤーへの方向ベクトル
		D3DXVECTOR3 toPlayer = pPlayer->GetPos() - pEnemy->GetPos();

		if (pPlayer)
		{
			toPlayer.y = 0.0f; // 水平方向のみ
			D3DXVec3Normalize(&toPlayer, &toPlayer);

			// 目標の角度を算出
			float targetYaw = atan2f(-toPlayer.x, -toPlayer.z);

			// 目的角度を設定（X,Zはそのまま）
			D3DXVECTOR3 rotDest = pEnemy->GetRot();
			rotDest.y = targetYaw;

			// 敵に目的角度を設定
			pEnemy->SetRotDest(rotDest);

			// 補間して回転
			pEnemy->UpdateRotation(0.5f);
		}
	}

	void OnUpdate(CEnemyLeader* pEnemy)override
	{
		// 視界内判定をするためにプレイヤーを取得
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		// 減速処理
		pEnemy->ApplyDeceleration();

		// リーダー敵の位置を取得
		D3DXVECTOR3 pos = pEnemy->GetPos();
		pos.y += EFFECT_OFFSET;// 少し上げる

		m_timer++;

		if (m_timer >= EFFECT_INTERVAL)
		{
			m_timer = 0;

			// 効果音ポップアップエフェクトの生成
			CSEPopupEffect::Create("data/TEXTURE/popup_question.png", pos, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 40);
		}

		// モーション中にプレイヤーが視界に入ったら
		if (pEnemy->IsPlayerInSight(pPlayer))
		{
			// 発見状態
			m_pMachine->ChangeState<CEnemyLeader_DiscoverState>();
			return;// 即移行
		}

		if (pEnemy->GetMotion()->IsCurrentMotionEnd(CEnemyLeader::DOUBT))
		{// 疑いモーションが終わっていたら

			// サブ敵が追跡状態だったら
			if (pEnemy->IsSubAction(CEnemy::AI_CHASE))
			{
				// 追跡状態
				m_pMachine->ChangeState<CEnemyLeader_ChaseState>();
				return;
			}

			// 音源の調査状態
			m_pMachine->ChangeState<CEnemyLeader_SoundInvestigateState>();
		}
	}

	void OnExit(CEnemyLeader* /*pEnemy*/)override
	{

	}

private:
	static constexpr float	EFFECT_OFFSET	= 40.0f;// エフェクト生成位置オフセット
	static constexpr int	EFFECT_INTERVAL = 15;	// エフェクト生成インターバル

	int m_timer;// エフェクト用生成タイマー
};

//*****************************************************************************
// リーダー敵の音源の調査状態
//*****************************************************************************
class CEnemyLeader_SoundInvestigateState :public StateBase<CEnemyLeader>
{
public:

	void OnStart(CEnemyLeader* pEnemy)override
	{
		// 音源の調査モーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemyLeader::SOUND_INVESTIGATE, 10);
	}

	void OnUpdate(CEnemyLeader* pEnemy)override
	{
		// 視界内判定をするためにプレイヤーを取得
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		// モーション中にプレイヤーが視界に入ったら
		if (pEnemy->IsPlayerInSight(pPlayer))
		{
			// 発見状態
			m_pMachine->ChangeState<CEnemyLeader_DiscoverState>();
			return;// 即移行
		}

		// サブ敵が追跡状態だったら
		if (pEnemy->IsSubAction(CEnemy::AI_CHASE))
		{
			// 追跡状態
			m_pMachine->ChangeState<CEnemyLeader_ChaseState>();
			return; // すぐに切り替え
		}

		// 音源のポイントに向かう
		D3DXVECTOR3 toTarget = pEnemy->GetLastHeardSoundPos() - pEnemy->GetPos();
		toTarget.y = 0;
		D3DXVec3Normalize(&toTarget, &toTarget);

		// 目標速度計算
		float moveSpeed = CEnemyLeader::INVESTIGATE_SPEED;
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

		currentMove.x += (targetMove.x - currentMove.x) * CEnemyLeader::ACCEL_RATE;
		currentMove.z += (targetMove.z - currentMove.z) * CEnemyLeader::ACCEL_RATE;

		// 補間後の速度をプレイヤーにセット
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
			// 巡回ポイントに到達したら警戒状態に戻す
			if (!pEnemy->IsInvestigating())
			{
				m_pMachine->ChangeState<CEnemyLeader_CautionState>();
			}
		}
	}

	void OnExit(CEnemyLeader* /*pEnemy*/)override
	{

	}

private:

};

//*****************************************************************************
// リーダー敵の埋蔵金の調査状態
//*****************************************************************************
class CEnemyLeader_TreasureInvestigateState :public StateBase<CEnemyLeader>
{
public:

	void OnStart(CEnemyLeader* pEnemy)override
	{
		// 埋蔵金の調査モーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemyLeader::TREASURE_INVESTIGATE, 10);
	}

	void OnUpdate(CEnemyLeader* pEnemy)override
	{
		// 視界内判定をするためにプレイヤーを取得
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		// モーション中にプレイヤーが視界に入ったら
		if (pEnemy->IsPlayerInSight(pPlayer))
		{
			// 追跡状態
			m_pMachine->ChangeState<CEnemyLeader_ChaseState>();
			return;// 即移行
		}

		// サブ敵が追跡状態だったら
		if (pEnemy->IsSubAction(CEnemy::AI_CHASE))
		{
			// 追跡状態
			m_pMachine->ChangeState<CEnemyLeader_ChaseState>();
			return; // すぐに切り替え
		}

		// 一番近い埋蔵金の場所に向かう
		D3DXVECTOR3 toTarget = pEnemy->GetNearestTreasurePos() - pEnemy->GetPos();
		toTarget.y = 0;
		D3DXVec3Normalize(&toTarget, &toTarget);

		// 目標速度計算
		float moveSpeed = CEnemyLeader::INVESTIGATE_SPEED;
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

		currentMove.x += (targetMove.x - currentMove.x) * CEnemyLeader::ACCEL_RATE;
		currentMove.z += (targetMove.z - currentMove.z) * CEnemyLeader::ACCEL_RATE;

		// 補間後の速度をプレイヤーにセット
		pEnemy->SetMove(currentMove);

		// 物理エンジンにセット
		btVector3 velocity = pEnemy->GetRigidBody()->getLinearVelocity();
		velocity.setX(currentMove.x);
		velocity.setZ(currentMove.z);
		pEnemy->GetRigidBody()->setLinearVelocity(velocity);

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
		if (pEnemy->HasReachedTreasure())
		{
			// 警戒状態
			m_pMachine->ChangeState<CEnemyLeader_CautionState>();
		}
	}

	void OnExit(CEnemyLeader* /*pEnemy*/)override
	{

	}

private:

};

//*****************************************************************************
// リーダー敵の警戒状態
//*****************************************************************************
class CEnemyLeader_CautionState :public StateBase<CEnemyLeader>
{
public:

	void OnStart(CEnemyLeader* pEnemy)override
	{
		// 警戒モーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemyLeader::CAUTION, 20);
	}

	void OnUpdate(CEnemyLeader* pEnemy)override
	{
		// 視界内判定をするためにプレイヤーを取得
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		// 特定のブロックに当たったか判定するため、ブロックマネージャーを取得する
		CBlockManager* pBlockManager = CGame::GetBlockManager();

		// 草むらの中か判定
		bool playerInGrass = pBlockManager->IsPlayerInGrass();

		// 減速処理
		pEnemy->ApplyDeceleration();

		// モーション中にプレイヤーが視界に入ったら
		if (pEnemy->IsPlayerInSight(pPlayer) && !playerInGrass)
		{
			// 追跡状態
			m_pMachine->ChangeState<CEnemyLeader_ChaseState>();
			return;// 即移行
		}

		// 疑い状態リクエストされていたら
		if (pEnemy->GetRequestedAction() == CEnemy::AI_DOUBT)
		{
			m_pMachine->ChangeState<CEnemyLeader_DoubtState>();
			return; // すぐに切り替え
		}

		// 命令状態リクエストされていたら
		if (pEnemy->GetRequestedAction() == CEnemy::AI_ORDER)
		{
			// 命令状態
			m_pMachine->ChangeState<CEnemyLeader_OrderState>();
			return; // すぐに切り替え
		}

		// 警戒モーションが終わっていたら
		if (pEnemy->GetMotion()->IsCurrentMotionEnd(CEnemyLeader::CAUTION))
		{
			// 一番近い巡回ポイントに戻す
			pEnemy->ReturnToPatrol();

			// 移動状態
			m_pMachine->ChangeState<CEnemyLeader_MoveState>();
		}
	}

	void OnExit(CEnemyLeader* /*pEnemy*/)override
	{

	}

private:

};

//*****************************************************************************
// リーダー敵の命令状態
//*****************************************************************************
class CEnemyLeader_OrderState :public StateBase<CEnemyLeader>
{
public:

	void OnStart(CEnemyLeader* pEnemy)override
	{
		// 命令モーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemyLeader::ORDER, 20);
	}

	void OnUpdate(CEnemyLeader* pEnemy)override
	{
		// 視界内判定のためにプレイヤーを取得
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		// 減速処理
		pEnemy->ApplyDeceleration();

		// 音源のポイントに向く
		D3DXVECTOR3 toTarget = pEnemy->GetLastHeardSoundPos() - pEnemy->GetPos();
		toTarget.y = 0;
		D3DXVec3Normalize(&toTarget, &toTarget);

		// 目標の角度を算出
		float targetYaw = atan2f(-toTarget.x, -toTarget.z);

		// 目的角度を設定（X,Zはそのまま）
		D3DXVECTOR3 rotDest = pEnemy->GetRot();
		rotDest.y = targetYaw;

		// 敵に目的角度を設定
		pEnemy->SetRotDest(rotDest);

		// 補間して回転
		pEnemy->UpdateRotation(0.05f);

		// 視界に入ったら
		if (pEnemy->IsPlayerInSight(pPlayer))
		{
			// 追跡状態
			m_pMachine->ChangeState<CEnemyLeader_ChaseState>();
			return;
		}

		// サブ敵が追跡状態だったら
		if (pEnemy->IsSubAction(CEnemy::AI_CHASE))
		{
			// 追跡状態
			m_pMachine->ChangeState<CEnemyLeader_ChaseState>();
			return;
		}

		// 命令モーションが終わっていたら
		if (pEnemy->GetMotion()->IsCurrentMotionEnd(CEnemyLeader::ORDER))
		{
			// 警戒状態
			m_pMachine->ChangeState<CEnemyLeader_CautionState>();
		}
	}

	void OnExit(CEnemyLeader* /*pEnemy*/)override
	{

	}

private:

};

//*****************************************************************************
// リーダー敵の追跡状態
//*****************************************************************************
class CEnemyLeader_ChaseState :public StateBase<CEnemyLeader>
{
public:

	void OnStart(CEnemyLeader* pEnemy)override
	{
		// 追跡モーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemyLeader::CHASE, 10);

		// 生成開始位置1を決める
		D3DXVECTOR3 startPos1 = pEnemy->GetPos();
		startPos1.y += ORBIT_OFFSET_TOP;

		// 生成開始位置2を決める
		D3DXVECTOR3 startPos2 = pEnemy->GetPos();
		startPos2.y += ORBIT_OFFSET_BOTTOM;

		// メッシュの軌跡の生成
		m_pOrbit = CMeshOrbit::Create(startPos1, startPos2, D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.8f), 50);
	}

	void OnUpdate(CEnemyLeader* pEnemy)override
	{
		// 軌跡の位置更新
		if (m_pOrbit)
		{
			// 生成開始位置1を決める
			D3DXVECTOR3 startPos1 = pEnemy->GetPos();
			startPos1.y += ORBIT_OFFSET_TOP;

			// 生成開始位置2を決める
			D3DXVECTOR3 startPos2 = pEnemy->GetPos();
			startPos2.y += ORBIT_OFFSET_BOTTOM;

			m_pOrbit->SetStartPos(startPos1, startPos2);
		}

		// モーションスピードを早くする
		pEnemy->GetMotion()->SetMotionSpeedRate(1.4f);

		// プレイヤーの取得
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		// プレイヤーの方向
		D3DXVECTOR3 toTarget = pPlayer->GetPos() - pEnemy->GetPos();
		toTarget.y = 0;
		float distance = D3DXVec3Length(&toTarget);
		D3DXVec3Normalize(&toTarget, &toTarget);

		CModel** models = pEnemy->GetModels();
		int num = pEnemy->GetNumModels();// モデル数

		for (int nCnt = 0; nCnt < num; nCnt++)
		{
			// アウトラインを赤にする
			models[nCnt]->SetOutlineColor(VEC4_RED);
		}

		// サブ敵が追跡中じゃなかったら
		if (!pEnemy->IsSubAction(CEnemy::AI_CHASE))
		{
			// サブ敵が追跡していなくて、距離が離れたら警戒へ
			if (distance > CAUTION_DISTANCE)
			{
				// 警戒状態
				m_pMachine->ChangeState<CEnemyLeader_CautionState>();
				return;
			}
		}

		// 特定のブロックに当たったか判定するため、ブロックマネージャーを取得する
		CBlockManager* pBlockManager = CGame::GetBlockManager();

		// 草むらの中か判定
		bool playerInGrass = pBlockManager->IsPlayerInGrass();

		// ステルス状態
		IsStealthSpec stealth;

		// プレイヤーがステルス中に草むらに潜んだら
		if (playerInGrass && stealth.IsSatisfiedBy(*pPlayer) && 
			!pEnemy->IsSubAction(CEnemy::AI_CHASE))
		{
			// 警戒状態へ移行
			m_pMachine->ChangeState<CEnemyLeader_CautionState>();
			return;
		}

		// 一定距離になったら
		if (distance < ATTACK_DISTANCE)
		{
			if (!pEnemy->IsCooldown()) // クールダウン中でなければ攻撃へ
			{
				// 近距離攻撃状態1へ
				m_pMachine->ChangeState<CEnemyLeader_CloseAttackState1>();
				return;
			}
			else
			{
				// クールダウン中は攻撃せず待機状態
				m_pMachine->ChangeState<CEnemyLeader_StandState>();
				return;
			}
		}

		// 目標速度計算
		float moveSpeed = CEnemyLeader::CHASE_SPEED;
		D3DXVECTOR3 targetMove = pEnemy->GetForward();

		if (targetMove.x != 0.0f || targetMove.z != 0.0f)
		{
			D3DXVec3Normalize(&targetMove, &targetMove);

			targetMove *= moveSpeed;
		}
		else
		{
			targetMove = INIT_VEC3;
		}

		// 現在速度との補間（イージング）
		D3DXVECTOR3 currentMove = pEnemy->GetMove();

		currentMove.x += (targetMove.x - currentMove.x) * CEnemyLeader::ACCEL_RATE;
		currentMove.z += (targetMove.z - currentMove.z) * CEnemyLeader::ACCEL_RATE;

		// 補間後の速度をプレイヤーにセット
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
	}

	void OnExit(CEnemyLeader* pEnemy)override
	{
		CModel** models = pEnemy->GetModels();
		int num = pEnemy->GetNumModels();// モデル数

		for (int nCnt = 0; nCnt < num; nCnt++)
		{
			// アウトラインを通常(黒色)に戻す
			models[nCnt]->SetOutlineColor(VEC4_BLACK);
		}

		// モーションスピードを通常に戻す
		pEnemy->GetMotion()->SetMotionSpeedRate(1.0f);
	}

private:
	static constexpr float ATTACK_DISTANCE		= 130.0f;	// 攻撃モーション移行距離
	static constexpr float CAUTION_DISTANCE		= 280.0f;	// 警戒モーション移行距離
	static constexpr float ORBIT_OFFSET_TOP		= 50.0f;	// 軌跡の上のオフセット位置
	static constexpr float ORBIT_OFFSET_BOTTOM	= 20.0f;	// 軌跡の下のオフセット位置

	CMeshOrbit* m_pOrbit = nullptr;
};

//*****************************************************************************
// リーダー敵の発見状態
//*****************************************************************************
class CEnemyLeader_DiscoverState :public StateBase<CEnemyLeader>
{
public:

	void OnStart(CEnemyLeader* pEnemy)override
	{
		m_timer = 0;

		// 発見モーション
		pEnemy->GetMotion()->StartBlendMotion(CEnemyLeader::DISCOVER, 10);

		// プレイヤー取得
		CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

		// プレイヤーへの方向ベクトル
		D3DXVECTOR3 toPlayer = pPlayer->GetPos() - pEnemy->GetPos();

		if (pPlayer)
		{
			toPlayer.y = 0.0f; // 水平方向のみ
			D3DXVec3Normalize(&toPlayer, &toPlayer);

			// 目標の角度を算出
			float targetYaw = atan2f(-toPlayer.x, -toPlayer.z);

			// 目的角度を設定（X,Zはそのまま）
			D3DXVECTOR3 rotDest = pEnemy->GetRot();
			rotDest.y = targetYaw;

			// 敵に目的角度を設定
			pEnemy->SetRotDest(rotDest);

			// 補間して回転
			pEnemy->UpdateRotation(0.5f);
		}
	}

	void OnUpdate(CEnemyLeader* pEnemy)override
	{
		// リーダー敵の位置を取得
		D3DXVECTOR3 pos = pEnemy->GetPos();
		pos.y += EFFECT_OFFSET;// 少し上げる

		// 減速処理
		pEnemy->ApplyDeceleration();

		m_timer++;

		if (m_timer >= EFFECT_INTERVAL)
		{
			m_timer = 0;

			// 効果音ポップアップエフェクトの生成
			CSEPopupEffect::Create("data/TEXTURE/popup_discover.png", pos, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 40);
		}

		// 発見モーションが終わっていたら
		if (pEnemy->GetMotion()->IsCurrentMotionEnd(CEnemyLeader::DISCOVER))
		{
			if (!pEnemy->IsCooldown())
			{
				// 追跡状態
				m_pMachine->ChangeState<CEnemyLeader_ChaseState>();
			}
			else
			{// クールダウン中は待機
				// 待機状態
				m_pMachine->ChangeState<CEnemyLeader_StandState>();
			}
		}
	}

	void OnExit(CEnemyLeader* /*pEnemy*/)override
	{

	}

private:
	static constexpr float	EFFECT_OFFSET	= 40.0f;	// エフェクト生成位置オフセット
	static constexpr int	EFFECT_INTERVAL = 10;		// エフェクト生成インターバル

	int m_timer;										// エフェクト用タイマー
};

#endif
