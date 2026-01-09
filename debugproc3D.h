//=============================================================================
//
// 3Dデバッグ表示処理 [debugproc3D.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _DEBUGPROC3D_H_// このマクロ定義がされていなかったら
#define _DEBUGPROC3D_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************

//*****************************************************************************
// 3Dデバッグクラス
//*****************************************************************************
class CDebugProc3D
{
public:
	CDebugProc3D();
	~CDebugProc3D();

	void Init(void);
	void Uninit(void);

	//*****************************************************************************
	// line描画関数
	//*****************************************************************************
	static void DrawLine3D(const D3DXVECTOR3& start, const D3DXVECTOR3& end, D3DXCOLOR color);
	static void DrawCapsuleCollider(btCapsuleShape* capsule, const btTransform& transform, D3DXCOLOR color);
	static void DrawBlockCollider(btRigidBody* rigidBody, D3DXCOLOR color);

private:
	static LPD3DXLINE m_pLine;   // ライン描画用オブジェクト

};

#endif