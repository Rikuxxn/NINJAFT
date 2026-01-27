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


//*****************************************************************************
// 前方宣言
//*****************************************************************************
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
	virtual float CalcSoundProbability(int targetCount)// 確率取得処理(0.0～1.0)
	{
		float x = (float)targetCount;

		return 1.0f / (1.0f + expf(-0.5f * (x - 5.0f)));
	}

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
		int makeSoundCount	= 0;	// 音を立てた回数
		int insightCount	= 0;	// 見つかった回数
	};

	void Update(CEnemy* pEnemy, CPlayer* pPlayer) override;
	void RecordPlayerAction(CEnemy* pEnemy, CPlayer* pPlayer) override;

private:
	static constexpr float	PROBABILITY_THRESHOLD	= 0.75f;	// 音の閾値
	static constexpr float	TRIGGER_DISTANCE		= 100.0f;	// 通常の距離
	static constexpr int	LOG_TIME				= 25;		// 記録時間
	static constexpr float	OFFSET_POS				= 40.0f;	// 生成オフセット位置
	static constexpr int	INTERVAL				= 15;		// インターバル

	PlayerBehaviorLog	m_log;									// プレイヤー行動構造体変数
	int					m_logTimer;								// 記録タイマー
	bool				m_prevInSight;							// 直前に視界に入ったか
	int					m_soundTimer;							// 波紋生成タイマー
	int					m_soundCount;							// 音発生数z
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
		int makeSoundCount	= 0;	// 音を立てた回数
		int insightCount	= 0;	// 見つかった回数
	};

	void Update(CEnemy* pEnemy, CPlayer* pPlayer) override;
	void RecordPlayerAction(CEnemy* pEnemy, CPlayer* pPlayer) override;

private:
	static constexpr float	PROBABILITY_THRESHOLD	= 0.75f;	// 音の閾値
	static constexpr float	DISTANCE_NORMAL			= 50.0f;	// 通常の距離
	static constexpr int	LOG_TIME				= 25;		// 記録時間

	PlayerBehaviorLog	m_log;									// プレイヤー行動構造体変数
	int					m_logTimer;								// 記録タイマー
	bool				m_prevInSight;							// 直前に視界に入ったか
};

#endif

