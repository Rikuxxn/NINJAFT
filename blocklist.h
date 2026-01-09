//=============================================================================
//
// ブロックリスト処理 [blocklist.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _BLOCKLIST_H_
#define _BLOCKLIST_H_

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "guage.h"
#include "blockmanager.h"

// 前方宣言
class CPlayer;
class CBlock;


//*****************************************************************************
// 壁ブロッククラス
//*****************************************************************************
class CWallBlock : public CBlock
{
public:
	CWallBlock();
	~CWallBlock();

	static TYPE GetStaticType(void) { return TYPE_WALL_01; }

	void Update(void);

private:
	static constexpr float resetDepth	= -810.0f;	// 位置リセットするZ位置
	static constexpr float bottomPosZ	= 540.0f;	// リセット位置
	static constexpr float moveSpeed	= 3.0f;		// 移動スピード

};

//*****************************************************************************
// 草地面ブロッククラス
//*****************************************************************************
class CGrassFloorBlock : public CBlock
{
public:
	CGrassFloorBlock();
	~CGrassFloorBlock();

	static TYPE GetStaticType(void) { return TYPE_GRASS_FLOOR; }

	void Update(void);

private:
	static constexpr float resetDepth	= -720.0f;	// 位置リセットするZ位置
	static constexpr float bottomPosZ	= 480.0f;	// リセット位置
	static constexpr float moveSpeed	= 3.0f;		// 移動スピード

};

//*****************************************************************************
// 灯籠ブロッククラス
//*****************************************************************************
class CTorchBlock : public CBlock
{
public:
	CTorchBlock();
	~CTorchBlock();

	static TYPE GetStaticType(void) { return TYPE_TORCH_01; }

	void Update(void);
	void UpdateLight(void) override;
	float GetDistMax(void) { return m_distMax; }

private:
	static constexpr float m_distMax = 110.0f;	// 判定距離

};

//*****************************************************************************
// 岩ブロッククラス
//*****************************************************************************
class CRockBlock : public CBlock
{
public:
	static TYPE GetStaticType(void) { return TYPE_ROCK; }

	int GetCollisionFlags(void) const override { return btCollisionObject::CF_NO_CONTACT_RESPONSE; }

private:

};

//*****************************************************************************
// 桜の木ブロッククラス
//*****************************************************************************
class CBlossomTreeBlock : public CBlock
{
public:
	CBlossomTreeBlock();
	~CBlossomTreeBlock();

	static TYPE GetStaticType(void) { return TYPE_BLOSSOM_TREE; }

	void Update(void);

private:
	static constexpr float resetDepth	= -1110.0f;	// 位置リセットするZ位置
	static constexpr float bottomPosZ	= 740.0f;	// リセット位置
	static constexpr float moveSpeed	= 3.0f;		// 移動スピード

};

//*****************************************************************************
// 埋蔵金ブロッククラス
//*****************************************************************************
class CBuriedTreasureBlock : public CBlock
{
public:
	CBuriedTreasureBlock();
	~CBuriedTreasureBlock();

	static TYPE GetStaticType(void) { return TYPE_BURIED_TREASURE; }

	HRESULT Init(void);
	void Update(void);
	bool IsGet(void)
	{
		// 一定数手に入れたら
		if (m_getCount >= GET_THRESHOLD)
		{
			return true;
		}

		return false;
	}

	static int GetTreasureCount(void) { return m_getCount; }

	int GetCollisionFlags(void) const override { return btCollisionObject::CF_NO_CONTACT_RESPONSE; }

private:
	static constexpr float	TRIGGER_DISTANCE		= 40.0f;	// 判定距離
	static constexpr int	SPAWN_TIME				= 180;		// 生成までの時間
	static constexpr int	GET_THRESHOLD			= 3;		// 取得数閾値
	static constexpr float	GUAGE_RATE				= 100.0f;	// ゲージの最大量
	static constexpr float	GUAGE_DECREASE_SPEED	= 0.22f;	// ゲージの減る量

	int			m_effectTimer;									// エフェクト生成タイマー
	C3DGuage*	m_pFrame;										// 枠
	C3DGuage*	m_pGuage;										// メインゲージ
	float		m_guageRate;									// ゲージの最大量
	float		m_guageDecreaseSpeed;							// ゲージの減る量
	bool		m_bUiActive;									// UIの表示・非表示フラグ
	static int	m_getCount;										// 取得数
};

//*****************************************************************************
// 扉ブロッククラス
//*****************************************************************************
class CDoorBlock : public CBlock
{
public:
	CDoorBlock();
	~CDoorBlock();

	static TYPE GetStaticType(void) { return TYPE_DOOR; }

	int GetCollisionFlags(void) const override { return btCollisionObject::CF_NO_CONTACT_RESPONSE; }

	void LoadFromJson(const json& b) override
	{
		CBlock::LoadFromJson(b);

		// 初期角度を m_rotY に度数で保存
		D3DXVECTOR3 rot = GetRot();             // ラジアン
		m_baseRotY = D3DXToDegree(rot.y);       // 度に変換して基準角度
		m_rotY = m_baseRotY;                    // 現在角度も同じ
	}

	void Update(void);
	bool DoorOpen(void)
	{
		// --- 埋蔵金ブロックが一定数取得されたか確認 ---
		auto buriedTreasureBlocks = CBlockManager::GetBlocksOfType<CBuriedTreasureBlock>();

		// 埋蔵金が存在しなかったらtrueを返す
		if (buriedTreasureBlocks.empty())
		{
			return true;
		}

		for (CBuriedTreasureBlock* treasure : buriedTreasureBlocks)
		{
			if (treasure->IsGet())
			{
				return true;
			}
		}

		return false;
	}

	bool IsOpen(void) { return m_isOpen; }

private:
	static constexpr float ROT_LIMIT = 90.0f;	// 回転角度
	static constexpr float ROT_SPEED = 0.5f;	// 回転スピード

	float		m_baseRotY;						// 基準の角度
	float		m_rotY;							// Y角度
	static bool m_isOpen;						// 開いたかどうか
	bool		m_prevOpen;						// 直前に開いたか
};

//*****************************************************************************
// 出口判定ブロッククラス
//*****************************************************************************
class CExitBlock : public CBlock
{
public:
	CExitBlock(int nPriority = 5);
	~CExitBlock();

	static TYPE GetStaticType(void) { return TYPE_GHOSTOBJECT; }

	void Update(void);
	bool IsHitPlayer(CPlayer* pPlayer);

	bool IsEscape(void) { return m_isEscape; }
	bool IsIn(void) { return m_isIn; }
	bool AvailableExit(void)
	{
		// --- 埋蔵金ブロックが一定数取得されたか確認 ---
		auto buriedTreasureBlocks = CBlockManager::GetBlocksOfType<CBuriedTreasureBlock>();

		// 埋蔵金が存在しなかったらtrueを返す
		if (buriedTreasureBlocks.empty())
		{
			return true;
		}

		for (CBuriedTreasureBlock* treasure : buriedTreasureBlocks)
		{
			if (treasure->IsGet())
			{
				return true;
			}
		}

		return false;
	}

private:
	static constexpr float TRIGGER_DISTACE = 110.0f;	// 判定距離

	bool m_isEscape;									// 脱出したかどうか
	bool m_isIn;										// 範囲内フラグ
};

//*****************************************************************************
// 門ブロッククラス
//*****************************************************************************
class CGateBlock : public CBlock
{
public:
	CGateBlock();
	~CGateBlock();

	static TYPE GetStaticType(void) { return TYPE_GATE; }

	void LoadFromJson(const json& b) override
	{
		CBlock::LoadFromJson(b);

		// 初期角度を m_rotY に度数で保存
		D3DXVECTOR3 rot = GetRot();             // ラジアン
		m_baseRotY = D3DXToDegree(rot.y);       // 度に変換して基準角度

		// 開始位置を保存
		m_startPosX = GetPos().x;
	}

	void Update(void);
	void GameGateUpdate(void);
	void MovieGateUpdate(void);

private:
	static constexpr int   MAX_STEP				= 6;		// 移動する段階数
	static constexpr float MOVE_UNIT			= 44.0f;	// 移動距離
	static constexpr float DELAY_TIME			= 780.0f;	// 遅延時間
	static constexpr float SHAKE_POWER			= 1.5f;		// 揺れ幅
	static constexpr float SHAKE_SPEED			= 20.0f;	// 揺れの細かさ
	static constexpr float SHAKE_DURATION		= 120.0f;	// 揺れる時間
	static constexpr float PRE_SHAKE_RANGE		= 0.02f;	// 予兆揺れ境界(～%手前から揺らす)
	static constexpr float SIDE_OFFSET			= 30.0f;	// 埃パーティクル生成位置オフセット
	static constexpr float CLOSE_DURATION		= 20.0f;	// 閉じるフレーム数

	float m_baseRotY;										// 基準の角度
	float m_startPosX;										// 開始位置
	float m_movieTime;										// 移動時間
	int   m_prevStep ;										// 直前の段階数
	bool  m_bClosing;										// 閉じているか
	float m_closeTimer;										// タイマー
	float m_fromX;											// 移動開始位置
	float m_toX;											// 移動到達位置

};

//*****************************************************************************
// ギアブロッククラス
//*****************************************************************************
class CGearBlock : public CBlock
{
public:
	CGearBlock();
	~CGearBlock();

	static TYPE GetStaticType(void) { return TYPE_GEAR; }

	void Update(void);
	void GameGearUpdate(void);
	void MovieGearUpdate(void);

private:
	static constexpr float ROT_SPEED	= 0.05f;	// 回転スピード
	static constexpr int DELAY_TIME		= 660;		// 回転までの遅延時間

	int	 m_turnTimer;								// 回転までの時間カウンター
	bool m_prevTimeEnd;								// タイマーが0になったか
};

#endif
