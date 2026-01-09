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
		DUSH,				// ダッシュ
		APPEARANCE,			// 出現
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
	void SetVisibleFlag(bool flag) { m_isVisible = flag; }
	void SetMove(D3DXVECTOR3 move) { m_move = move; }

	//*****************************************************************************
	// getter関数
	//*****************************************************************************
	D3DXVECTOR3 GetPos(void) { return m_pos; }
	D3DXVECTOR3 GetRot(void) { return m_rot; }
	D3DXVECTOR3 GetMove(void) { return m_move; }
	D3DXVECTOR3 GetForward(void);
	CMotion* GetMotion(void) { return m_pMotion; }
	bool IsVisible(void) { return m_isVisible; }

private:
	static constexpr int MAX_PARTS = 32;// 最大パーツ数

	D3DXVECTOR3 m_pos;					// 位置
	D3DXVECTOR3 m_rot;					// 向き
	D3DXVECTOR3 m_size;					// サイズ
	D3DXVECTOR3 m_move;					// 移動量
	D3DXMATRIX	m_mtxWorld;				// ワールドマトリックス
	CModel*		m_apModel[MAX_PARTS];	// モデル(パーツ)へのポインタ
	CShadowS*	m_pShadowS;				// ステンシルシャドウへのポインタ
	CMotion*	m_pMotion;				// モーションへのポインタ
	int			m_nNumModel;			// モデル(パーツ)の総数
	int			m_motionType;			// モーションのタイプ
	bool		m_isVisible;			// 可視フラグ

};

#endif
