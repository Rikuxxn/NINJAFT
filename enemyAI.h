//=============================================================================
//
// 敵の学習AI処理 [enemyAI.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _ENEMYAI_H_
#define _ENEMYAI_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************


// --- 前方宣言 ---
class CPlayer;
class CEnemy;

//*****************************************************************************
// 敵学習AI基底クラス
//*****************************************************************************
class IEnemyAI
{
public:
	IEnemyAI() {}
	virtual ~IEnemyAI() {}

	virtual void Update(class CEnemy* enemy, CPlayer* player) = 0;
	virtual void RecordPlayerAction(CEnemy* pEnemy, CPlayer* pPlayer) = 0;
};

//*****************************************************************************
// リーダーの学習AIクラス
//*****************************************************************************
class CEnemyAI_Leader : public IEnemyAI
{
public:
	CEnemyAI_Leader();
	~CEnemyAI_Leader();

	// プレイヤーの行動記録構造体
	struct PlayerBehaviorLog
	{
		int makeSoundCount = 0;	// 音を立てた回数
		int insightCount = 0;	// 見つかった回数
	};

	void Update(CEnemy* pEnemy, CPlayer* pPlayer) override;
	void RecordPlayerAction(CEnemy* pEnemy, CPlayer* pPlayer) override;
	float CalcSoundProbability(int targetCount);

private:
	PlayerBehaviorLog m_log;
	int m_logTimer;				// 記録タイマー
	bool m_prevInSight;			// 直前に視界に入ったか
};

//*****************************************************************************
// サブの学習AIクラス
//*****************************************************************************
class CEnemyAI_Sub : public IEnemyAI
{
public:
	CEnemyAI_Sub();
	~CEnemyAI_Sub();

	// プレイヤーの行動記録構造体
	struct PlayerBehaviorLog
	{
		int makeSoundCount = 0;	// 音を立てた回数
		int insightCount = 0;	// 見つかった回数
	};

	void Update(CEnemy* pEnemy, CPlayer* pPlayer) override;
	void RecordPlayerAction(CEnemy* pEnemy, CPlayer* pPlayer) override;
	float CalcSoundProbability(void);

private:
	PlayerBehaviorLog m_log;
	int m_logTimer;				// 記録タイマー
	bool m_prevInSight;			// 直前に視界に入ったか
};

#endif

