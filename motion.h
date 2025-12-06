//=============================================================================
//
// モーション処理 [motion.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _MOTION_H_// このマクロ定義がされていなかったら
#define _MOTION_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "model.h"

//*****************************************************************************
// モーションクラス
//*****************************************************************************
class CMotion
{
public:
	CMotion();
	~CMotion();

	static CMotion* Load(const char* pFilepath, CModel* pModel[], int& nNumModel, int nMaxMotion);
	void LoadModelInfo(FILE* pFile, char* aString, CModel* pModel[], int& nNumModel, int& nIdx);
	void LoadCharacterSet(FILE* pFile, char* aString, CModel* pModel[], int nNumModel, int parentIdx[]);
	void LoadMotionSet(FILE* pFile, char* aString, CMotion* pMotion, int& nCntMotion, int nMaxMotion);
	void Update(CModel** pModel, int& nNumModel);
	void StartBlendMotion(int  motionTypeBlend, int nFrameBlend);
	void SetMotion(int  motionType);
	void AdvanceKeyCounter(int motionType, int& nKey, int& nCounter, bool bLoop);

	bool IsCurrentMotion(int motionType) const;
	bool IsCurrentMotionEnd(int motionType) const;
	int GetMotionType(void) { return m_motionType; }

	bool IsAttacking(int motionType, int startKey, int endKey, int startFrame, int endFrame) const;
	bool IsAttacking(int motionType) const;
	float GetMotionRate(void) const;

private:
	static constexpr int MAX_WORD	= 1024;	// 最大文字数
	static constexpr int MAX_PARTS	= 32;	// 最大パーツ数
	static constexpr int MAX_KEY	= 256;	// 最大キー数
	static constexpr int MAX_MOTION = 20;	// モーションの最大数

	//*************************************************************************
	// キー構造体
	//*************************************************************************
	typedef struct
	{
		float fPosX;							// 位置(X)
		float fPosY;							// 位置(Y)
		float fPosZ;							// 位置(Z)
		float fRotX;							// 向き(X)
		float fRotY;							// 向き(Y)
		float fRotZ;							// 向き(Z)
	}KEY;

	//*************************************************************************
	// 補間関数(位置)
	//*************************************************************************
	inline D3DXVECTOR3 LerpPos(const KEY& a, const KEY& b, float t)
	{
		return D3DXVECTOR3(
			a.fPosX + (b.fPosX - a.fPosX) * t,
			a.fPosY + (b.fPosY - a.fPosY) * t,
			a.fPosZ + (b.fPosZ - a.fPosZ) * t
		);
	}

	//*************************************************************************
	// 補間関数(向き)
	//*************************************************************************
	inline D3DXVECTOR3 LerpRot(const KEY& a, const KEY& b, float t)
	{
		auto delta = [](float from, float to)
		{
			float d = to - from;
			if (d > D3DX_PI) d -= D3DX_PI * 2.0f;
			else if (d < -D3DX_PI) d += D3DX_PI * 2.0f;
			return d;
		};

		return D3DXVECTOR3(
			a.fRotX + delta(a.fRotX, b.fRotX) * t,
			a.fRotY + delta(a.fRotY, b.fRotY) * t,
			a.fRotZ + delta(a.fRotZ, b.fRotZ) * t
		);
	}

	//*************************************************************************
	// キー情報の構造体
	//*************************************************************************
	typedef struct
	{
		int nFrame;								// 再生フレーム
		KEY aKey[MAX_PARTS];					// 各パーツのキー要素
	}KEY_INFO;

	//*************************************************************************
	// モーション情報の構造体
	//*************************************************************************
	typedef struct
	{
		bool bLoop;								// ループするかどうか
		int nNumKey;							// キーの総数
		KEY_INFO aKeyInfo[MAX_KEY];				// キー情報
		int startKey, startFrame;
	}MOTION_INFO;

	MOTION_INFO m_aMotionInfo[MAX_MOTION];		// モーション情報
	int  m_motionType;							// モーションの種類
	int m_nNumMotion;							// モーション総数
	bool m_bLoopMotion;							// ループするかどうか
	int m_nNumKey;								// キーの総数
	int m_nKey;									// 現在のキーNo.
	int m_nCounterMotion;						// モーションのカウンター

	bool m_bFinishMotion;						// 現在のモーションが終了しているかどうか
	bool m_bBlendMotion;						// ブレンドモーションがあるかどうか
	int  m_motionTypeBlend;						// ブレンドモーションがあるかどうか
	bool m_bLoopMotionBlend;					// ブレンドモーションがループするかどうか
	int m_nNumKeyBlend;							// ブレンドモーションのキー数
	int m_nKeyBlend;							// ブレンドモーションの現在のキーNo.
	int m_nCounterMotionBlend;					// ブレンドモーションのカウンター
	int m_nFrameBlend;							// ブレンドのフレーム数(何フレームかけてブレンドするか)
	int m_nCounterBlend;						// ブレンドカウンター
};

#endif