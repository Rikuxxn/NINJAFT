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
// 木箱ブロッククラス
//*****************************************************************************
class CWoodBoxBlock : public CBlock
{
public:
	CWoodBoxBlock();
	~CWoodBoxBlock();

	static TYPE GetStaticType(void) { return TYPE_WOODBOX; }

	HRESULT Init(void);
	void Update(void);
	void Respawn(D3DXVECTOR3 resPos) override;

	bool IsDynamicBlock(void) const override { return true; }
	btVector3 GetAngularFactor(void) const { return btVector3(1.0f, 1.0f, 1.0f); }
	btScalar GetMass(void) const { return MASS; }  // 質量の取得

private:
	static constexpr float MASS = 1.0f;
	static constexpr float RESPAWN_HEIGHT = -800.0f;

	D3DXVECTOR3 m_ResPos;	// リスポーン位置
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
	static constexpr float m_distMax = 110.0f;

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
// 水ブロッククラス
//*****************************************************************************
class CWaterBlock : public CBlock
{
public:
	CWaterBlock(int nPriority = 4);
	~CWaterBlock();

	static TYPE GetStaticType(void) { return TYPE_WATER; }

	void Update(void);
	bool IsHit(void) { return m_isHit; }

private:
	static constexpr int SPAWN_TIME = 7;		// 生成間隔(フレーム)
	static constexpr float SPAWN_RADIUS = 10.0f;// 生成半径

	int m_counter;	// 生成カウンター
	bool m_isHit;	// 当たっているか
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
	static constexpr float TRIGGER_DISTANCE = 40.0f;	// 判定距離
	static constexpr int SPAWN_TIME = 180;				// 生成までの時間
	static constexpr int GET_THRESHOLD = 2;				// 取得数閾値
	static constexpr float GUAGE_RATE = 100.0f;			// ゲージの最大量
	static constexpr float GUAGE_DECREASE_SPEED = 0.23f;// ゲージの減る量

	int m_effectTimer;									// エフェクト生成タイマー
	C3DGuage* m_pFrame;									// 枠
	C3DGuage* m_pGuage;									// メインゲージ
	float m_guageRate;									// ゲージの最大量
	float m_guageDecreaseSpeed;							// ゲージの減る量
	bool m_bUiActive;									// UIの表示・非表示フラグ
	static int m_getCount;								// 取得数
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

		for (CBuriedTreasureBlock* treasure : buriedTreasureBlocks)
		{
			if (treasure->IsGet() || buriedTreasureBlocks.empty())
			{
				return true;
			}
		}

		return false;
	}

	bool IsOpen(void) { return m_isOpen; }

private:
	static constexpr float ROT_LIMIT = 90.0f;
	static constexpr float ROT_SPEED = 0.5f;

	float m_baseRotY;		// 基準の角度
	float m_rotY;			// Y角度
	static bool m_isOpen;	// 開いたかどうか
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
	bool AvailableExit(void)
	{
		// --- ドアブロックが開いたか確認 ---
		auto doorBlocks = CBlockManager::GetBlocksOfType<CDoorBlock>();

		for (CDoorBlock* door : doorBlocks)
		{
			if (door->IsOpen())
			{
				return true;
			}
		}

		return false;
	}

private:
	static constexpr float TRIGGER_DISTACE = 100.0f;

	bool m_isEscape;	// 脱出したかどうか
};

#endif
