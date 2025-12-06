//=============================================================================
//
// ダミープレイヤー処理 [dummyPlayer.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _DUMMYPLAYER_H_// このマクロ定義がされていなかったら
#define _DUMMYPLAYER_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "object.h"
#include "model.h"
#include "shadowS.h"
#include "motion.h"

//*****************************************************************************
// ダミープレイヤークラス
//*****************************************************************************
class CDummyPlayer : public CObject
{
public:
	CDummyPlayer(int nPriority = 2);
	~CDummyPlayer();

	// ダミープレイヤーモーションの種類
	enum DUMMY_MOTION
	{
		NEUTRAL = 0,		// 待機
		MAX
	};

	static CDummyPlayer* Create(D3DXVECTOR3 pos, D3DXVECTOR3 rot, int motionType);
	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void Draw(void);

	//*****************************************************************************
	// setter関数
	//*****************************************************************************
	void SetPos(D3DXVECTOR3 pos) { m_pos = pos; }
	void SetRot(D3DXVECTOR3 rot) { m_rot = rot; }
	void SetMotionType(int type) { m_motionType = type; }

private:
	static constexpr int MAX_PARTS = 32;		// 最大パーツ数

	D3DXVECTOR3 m_pos;					// 位置
	D3DXVECTOR3 m_rot;					// 向き
	D3DXVECTOR3 m_size;					// サイズ
	D3DXMATRIX m_mtxWorld;				// ワールドマトリックス
	CModel* m_apModel[MAX_PARTS];		// モデル(パーツ)へのポインタ
	CShadowS* m_pShadowS;				// ステンシルシャドウへのポインタ
	CMotion* m_pMotion;					// モーションへのポインタ
	int m_nNumModel;					// モデル(パーツ)の総数
	int m_motionType;					// モーションのタイプ

};

#endif
