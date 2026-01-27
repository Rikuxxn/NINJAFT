//=============================================================================
//
// カメラ処理 [camera.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _CAMERA_H_// このマクロ定義がされていなかったら
#define _CAMERA_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// カメラクラス
//*****************************************************************************
class CCamera
{
public:
	CCamera();
	~CCamera();

	// ムービーカメラ構造体
	struct CameraKeyFrame
	{
		D3DXVECTOR3 posV;
		D3DXVECTOR3 posR;
		D3DXVECTOR3 rot;
		float       distance;
		int         duration;	// このキーへ移動するまでのフレーム数
		int			delay;		// このキーで待つフレーム（開始用）
	};

	// カメラの種類
	typedef enum
	{
		MODE_EDIT = 0,	// エディターカメラ
		MODE_GAME,		// ゲームカメラ
		MODE_DIRECTION,	// 演出カメラ
		MODE_MOVIE,		// ムービーカメラ
		MODE_MAX
	}MODE;

	HRESULT Init(void);
	void Uninit(void);
	void Update(void);
	void SetCamera(void);
	void EditCamera(void);
	void GameCamera(void);
	void AdjustCameraPosition(const D3DXVECTOR3& playerPos);
	void DirectionCamera(int nTimer);
	void MovieCamera(void);
	void CameraShake(void);
	void TriggerEventShake(float power, float duration);

	//*****************************************************************************
	// flagment関数
	//*****************************************************************************
	void IsDirection(bool flag) { m_isDirection = flag; }

	//*****************************************************************************
	// setter関数
	//*****************************************************************************
	void SetPosV(D3DXVECTOR3 posV) { m_posV = posV; }
	void SetPosR(D3DXVECTOR3 posR) { m_posR = posR; }
	void SetRot(D3DXVECTOR3 rot) { m_rot = rot; }
	void SetDis(float fDistance) { m_fDistance = fDistance; }
	int  SetTimer(int nTime) { return m_nTimer = nTime * 60; }
	void SetCamMode(MODE mode) { m_Mode = mode; }
	void SetCamDirection(int nTimer, D3DXVECTOR3 posV, D3DXVECTOR3 posR, D3DXVECTOR3 rot);
	void SetCamParameter(D3DXVECTOR3 posV, D3DXVECTOR3 posR, D3DXVECTOR3 rot, float fDistance);
	void SetMovieCamera(const std::vector<CameraKeyFrame>& keys);
	void SetShakeTargetPos(D3DXVECTOR3 targetpos) { m_targetPos = targetpos; }
	void SetCameraShake(bool flag) { m_isCameraShakeOn = flag; }

	//*****************************************************************************
	// getter関数
	//*****************************************************************************
	D3DXVECTOR3 GetRot(void) { return m_rot; }			// カメラの角度の取得
	MODE GetMode(void) { return m_Mode; }
	D3DXVECTOR3 GetForward(void) const;
	D3DXVECTOR3 GetPosV(void) { return m_posV; }
	D3DXVECTOR3 GetPosR(void) { return m_posR; }
	bool GetDirection(void) { return m_isDirection; }

private:
	void NormalizeRotX(void);// 角度の正規化 X軸
	void NormalizeRotY(void);// 角度の正規化 Y軸

private:
	static constexpr float SHAKE_DISTANCE		= 1300.0f;	// 振動する距離
	static constexpr float MOUSE_SENSITIVITY	= 0.004f;	// マウス感度
	static constexpr float GAMECAM_DISTANCE		= 580.0f;	// ゲームカメラ時の距離
	static constexpr float PI_HALF				= 1.57f;	// 円周率の半分

	D3DXVECTOR3 m_posV;					// 視点
	D3DXVECTOR3 m_posVDest;				// 目的の視点
	D3DXVECTOR3 m_posR;					// 注視点
	D3DXVECTOR3 m_posRDest;				// 目的の注視点
	D3DXVECTOR3 m_vecU;					// 上方向ベクトル
	D3DXMATRIX	m_mtxProjection;		// プロジェクションマトリックス
	D3DXMATRIX	m_mtxView;				// ビューマトリックス
	D3DXVECTOR3 m_rot;					// 向き
	D3DXVECTOR3 m_targetPos;			// 対象の位置(カメラ振動用)
	D3DXVECTOR3 m_shakeOffset;			// 最終的に加えるオフセット
	float		m_eventShakeTime;		// 残りのイベントシェイク時間
	float		m_eventShakePower;		// イベントシェイクの最大強さ
	float		m_eventShakeDuration;	// シェイクの減衰
	float		m_fDistance;			// 視点から注視点の距離
	MODE		m_Mode;					// カメラのモード
	int			m_nDirectionCamTimer;	// 演出カメラへの移行時間
	int			m_nTimer;				// タイマー
	bool		m_isDirection;			// 演出カメラかどうか
	bool		m_isCameraShakeOn;		// カメラシェイクのON/OFF

	int			m_currentKey;			// ムービーカメラの現在のキー
	int			m_directionFrame;		// ムービーカメラのカウンタ
	int			m_delayFrame;			// 遅延
	std::vector<CameraKeyFrame> m_camKeys;
};

#endif