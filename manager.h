//=============================================================================
//
// マネージャー処理 [manager.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _MANAGER_H_// このマクロ定義がされていなかったら
#define _MANAGER_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "renderer.h"
#include "input.h"
#include "sound.h"
#include "texture.h"
#include "camera.h"
#include "light.h"
#include "scene.h"
#include "fade.h"
#include "pausemanager.h"

//*****************************************************************************
// マネージャークラス
//*****************************************************************************
class CManager
{
public:
	CManager();
	~CManager();

	HRESULT Init(HINSTANCE hInstance, HWND hWnd);
	void Uninit(void);
	void Update(void);
	void Draw(void);

	static CRenderer* GetRenderer(void) { return m_pRenderer; }
	static CInputKeyboard* GetInputKeyboard(void) { return m_pInputKeyboard; }
	static CInputJoypad* GetInputJoypad(void) { return m_pInputJoypad; }
	static CInputMouse* GetInputMouse(void) { return m_pInputMouse; }
	static CSound* GetSound(void) { return m_pSound; }
	static CTexture* GetTexture(void) { return m_pTexture; }
	static CCamera* GetCamera(void) { return m_pCamera; }
	static CFade* GetFade(void) { return m_pFade; }
	static btDiscreteDynamicsWorld* GetPhysicsWorld(void) { return m_pDynamicsWorld.get(); }
	int GetFPS(int fps) { return m_fps = fps; };
	int GetFPSCnt(void) { return m_fps; }

	static void SetMode(CScene::MODE mode);
	static CScene::MODE GetMode(void);
	static void OnDeviceReset(void);
	static void ReleaseThumbnail(void);
	static void ResetThumbnail(void);

private:
	int													 m_fps;						// FPS値
	static CRenderer*									 m_pRenderer;				// レンダラーへのポインタ
	static CInputKeyboard*								 m_pInputKeyboard;			// キーボードへのポインタ
	static CInputJoypad*								 m_pInputJoypad;			// ジョイパッドへのポインタ
	static CInputMouse*									 m_pInputMouse;				// マウスへのポインタ
	static CSound*										 m_pSound;					// サウンドへのポインタ
	static CTexture*									 m_pTexture;				// テクスチャへのポインタ
	static CCamera*										 m_pCamera;					// カメラへのポインタ
	static CFade*										 m_pFade;					// フェードへのポインタ
	static CScene*										 m_pScene;					// シーンへのポインタ
	std::unique_ptr<btBroadphaseInterface>				 m_pBroadphase;				// 衝突判定のクラスへのポインタ
	std::unique_ptr<btDefaultCollisionConfiguration>	 m_pCollisionConfiguration;	// 衝突検出の設定を管理するクラスへのポインタ
	std::unique_ptr<btCollisionDispatcher>				 m_pDispatcher;				// 実際に衝突判定処理を実行するクラスへのポインタ
	std::unique_ptr<btSequentialImpulseConstraintSolver> m_pSolver;					// 物理シミュレーションの制約ソルバーへのポインタ
	static std::unique_ptr<btDiscreteDynamicsWorld>		 m_pDynamicsWorld;			// 物理世界を管理するクラスへのポインタ
};

#endif