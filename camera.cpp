//=============================================================================
//
// カメラ処理 [camera.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "camera.h"
#include "renderer.h"
#include "manager.h"
#include "game.h"
#include "player.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CCamera::CCamera()
{
	// 値のクリア
	m_posV					= INIT_VEC3;// 視点
	m_posVDest				= INIT_VEC3;// 目的の視点
	m_posR					= INIT_VEC3;// 注視点
	m_posRDest				= INIT_VEC3;// 目的の注視点
	m_vecU					= INIT_VEC3;// 上方向ベクトル
	m_mtxProjection			= {};		// プロジェクションマトリックス
	m_mtxView				= {};		// ビューマトリックス
	m_rot					= INIT_VEC3;// 向き
	m_targetPos				= INIT_VEC3;// カメラ振動用の対象位置
	m_shakeOffset			= INIT_VEC3;// 最終的に加えるオフセット
	m_eventShakeTime		= 0.0f;     // 残りのイベントシェイク時間
	m_eventShakePower		= 0.0f;     // イベントシェイクの最大強さ
	m_eventShakeDuration	= 0.0f;		// シェイクの減衰
	m_fDistance				= 0.0f;		// 視点から注視点の距離
	m_nDirectionCamTimer	= 0;		// 演出カメラ時間
	m_nTimer				= 0;		// タイマー
	m_isDirection			= false;	// 演出カメラかどうか
	m_isCameraShakeOn		= false;	// カメラシェイクのON/OFF
	m_currentKey			= 0;		// ムービーカメラの現在のキー
	m_directionFrame		= 0;		// ムービーカメラのカウンタ
	m_delayFrame			= 0;		// 遅延
#ifdef _DEBUG
	m_Mode = MODE_EDIT;					// カメラのモード
#else
	m_Mode = MODE_GAME;
#endif
}
//=============================================================================
// デストラクタ
//=============================================================================
CCamera::~CCamera()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CCamera::Init(void)
{	
	m_posV = D3DXVECTOR3(0.0f, 80.0f, -540.0f);
	m_posR = D3DXVECTOR3(0.0f, 80.0f, 0.0f);
	m_vecU = D3DXVECTOR3(0.0f, 1.0f, 0.0f);// 固定でいい
	m_rot = D3DXVECTOR3(0.0f, D3DX_PI, 0.0f);
	
#ifdef _DEBUG
	m_Mode = MODE_EDIT;									// カメラのモード

	m_fDistance = sqrtf(
		((m_posV.x - m_posR.x) * (m_posV.x - m_posR.x)) +
		((m_posV.y - m_posR.y) * (m_posV.y - m_posR.y)) +
		((m_posV.z - m_posR.z) * (m_posV.z - m_posR.z)));

#else
	m_Mode = MODE_GAME;

	m_fDistance = 220.0f;

#endif

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CCamera::Uninit(void)
{

}
//=============================================================================
// 更新処理
//=============================================================================
void CCamera::Update(void)
{
	// リスナーの位置の更新
	CManager::GetSound()->UpdateListener(m_posV);

#ifdef _DEBUG
	// キーボードの取得
	CInputKeyboard* pInputKeyboard = CManager::GetInputKeyboard();

	if (m_Mode == MODE_EDIT && pInputKeyboard->GetTrigger(DIK_C) == true)
	{
		// ゲームカメラ
		m_Mode = MODE_GAME;
	}
	else if (m_Mode == MODE_GAME && pInputKeyboard->GetTrigger(DIK_C) == true)
	{
		// エディターカメラ
		m_Mode = MODE_EDIT;
	}
#endif
	switch (m_Mode)
	{
	case MODE_EDIT:

#ifdef _DEBUG
		//if (CManager::GetMode() == MODE_TITLE)
		//{
		//	return;
		//}

		// エディターカメラの処理
		EditCamera();
#endif
		break;
	case MODE_GAME:
		// ゲームのカメラ処理
		GameCamera();
		break;

	case MODE_DIRECTION:
		// 演出カメラの処理
		DirectionCamera(m_nTimer);
		break;

	case MODE_MOVIE:
		// ムービーカメラの処理
		MovieCamera();
		break;
	}

	CameraShake();// カメラシェイク処理

}
//=============================================================================
// カメラの設定処理
//=============================================================================
void CCamera::SetCamera(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// ビューマトリックスの初期化
	D3DXMatrixIdentity(&m_mtxView);

	// ビューマトリックスの作成
	D3DXMatrixLookAtLH(&m_mtxView,
		&m_posV,
		&m_posR,
		&m_vecU);

	// ビューマトリックスの設定
	pDevice->SetTransform(D3DTS_VIEW, &m_mtxView);

	// プロジェクションマトリックスの初期化
	D3DXMatrixIdentity(&m_mtxProjection);

	// プロジェクションマトリックスの作成
	D3DXMatrixPerspectiveFovLH(&m_mtxProjection,
		D3DXToRadian(80.0f),						// 視野角
		(float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, // アスペクト比
		1.0f,										// 近クリップ面
		3500.0f);									// 遠クリップ面

	// プロジェクションマトリックスの設定
	pDevice->SetTransform(D3DTS_PROJECTION, &m_mtxProjection);
}
//=============================================================================
// エディターカメラの処理
//=============================================================================
void CCamera::EditCamera(void)
{
	// キーボードの取得
	CInputKeyboard* pInputKeyboard = CManager::GetInputKeyboard();

	// マウスの取得
	CInputMouse* pInputMouse = CManager::GetInputMouse();

	// マウスカーソルを表示する
	pInputMouse->SetCursorVisibility(true);

	// 現在のカーソル位置を取得
	POINT cursorPos;
	GetCursorPos(&cursorPos);

	// 前フレームからのマウス移動量を取得
	static POINT prevCursorPos = { cursorPos.x, cursorPos.y };
	float deltaX = (float)(cursorPos.x - prevCursorPos.x);
	float deltaY = (float)(cursorPos.y - prevCursorPos.y);

	// 現在のカーソル位置を保存（次のフレームでの比較用）
	prevCursorPos = cursorPos;

	deltaX *= MOUSE_SENSITIVITY;
	deltaY *= MOUSE_SENSITIVITY;

	//====================================
	// マウスホイールでズームイン・アウト
	//====================================
	int wheel = pInputMouse->GetWheel();
	const float zoomSpeed = 15.0f;

	if (wheel != 0)
	{
		m_fDistance -= wheel * zoomSpeed;

		// カメラ距離制限
		if (m_fDistance < 100.0f)
		{
			m_fDistance = 100.0f;
		}
		if (m_fDistance > 800.0f)
		{
			m_fDistance = 800.0f;
		}
	}

	if (pInputKeyboard->GetPress(DIK_LALT) && pInputMouse->GetPress(0)) // 左クリック押しながらマウス移動 → 視点回転
	{
		m_rot.y += deltaX; // 水平回転
		m_rot.x += deltaY; // 垂直回転

		//角度の正規化
		NormalizeRotY();

		// 垂直回転の制限
		if (m_rot.x > PI_HALF)
		{
			m_rot.x = PI_HALF;
		}

		if (m_rot.x < -PI_HALF)
		{
			m_rot.x = -PI_HALF;
		}

		// 視点の更新（カメラの方向を適用）
		m_posV.x = m_posR.x + sinf(m_rot.y) * cosf(m_rot.x) * m_fDistance;
		m_posV.y = m_posR.y + sinf(m_rot.x) * m_fDistance;
		m_posV.z = m_posR.z + cosf(m_rot.y) * cosf(m_rot.x) * m_fDistance;

	}
	else if (pInputMouse->GetPress(1)) // 右クリック押しながらマウス移動 → 注視点回転
	{
		m_rot.y += deltaX; // 水平回転
		m_rot.x += deltaY; // 垂直回転

		//角度の正規化
		NormalizeRotY();

		// 垂直回転の制限
		if (m_rot.x > PI_HALF)
		{
			m_rot.x = PI_HALF;
		}
		if (m_rot.x < -PI_HALF)
		{
			m_rot.x = -PI_HALF;
		}

		// 注視点の更新
		m_posR.x = m_posV.x - sinf(m_rot.y) * cosf(m_rot.x) * m_fDistance;
		m_posR.y = m_posV.y - sinf(m_rot.x) * m_fDistance;
		m_posR.z = m_posV.z - cosf(m_rot.y) * cosf(m_rot.x) * m_fDistance;
	}
	else
	{
		// 入力がない場合でもズーム反映のために視点を更新
		m_posV.x = m_posR.x + sinf(m_rot.y) * cosf(m_rot.x) * m_fDistance;
		m_posV.y = m_posR.y + sinf(m_rot.x) * m_fDistance;
		m_posV.z = m_posR.z + cosf(m_rot.y) * cosf(m_rot.x) * m_fDistance;
	}

	// 注視点の更新
	m_posR.x = m_posV.x - sinf(m_rot.y) * cosf(m_rot.x) * m_fDistance;
	m_posR.y = m_posV.y - sinf(m_rot.x) * m_fDistance;
	m_posR.z = m_posV.z - cosf(m_rot.y) * cosf(m_rot.x) * m_fDistance;
}
//=============================================================================
// ゲームカメラの処理
//=============================================================================
void CCamera::GameCamera(void)
{
	if ((CManager::GetMode() != CScene::MODE_GAME && CManager::GetMode() != CScene::MODE_TUTORIAL) || 
		m_Mode == MODE_DIRECTION)
	{
		return;
	}

	m_posR = D3DXVECTOR3(0.0f, -55.8f, -30.0f);

	// 見下ろし用角度（少し斜め）
	m_rot.x = 0.89f;
	m_rot.y = D3DX_PI;

	// 距離設定
	m_fDistance = GAMECAM_DISTANCE;

	// カメラ位置計算
	m_posV.x = m_posR.x + sinf(m_rot.y) * cosf(m_rot.x) * m_fDistance;
	m_posV.y = m_posR.y + sinf(m_rot.x) * m_fDistance;
	m_posV.z = m_posR.z + cosf(m_rot.y) * cosf(m_rot.x) * m_fDistance;
}
//=============================================================================
// カメラの位置補正(壁貫通をなくす)処理
//=============================================================================
void CCamera::AdjustCameraPosition(const D3DXVECTOR3& playerPos)
{
	// プレイヤーの頭位置を注視点とする
	D3DXVECTOR3 playerEye = playerPos;
	playerEye.y += 60.0f; // プレイヤーの頭の高さ
	m_posR = playerEye;

	// カメラの理想位置
	D3DXVECTOR3 offsetFromPlayer;
	offsetFromPlayer.x = sinf(m_rot.y) * cosf(m_rot.x) * m_fDistance;
	offsetFromPlayer.y = sinf(m_rot.x) * m_fDistance + 20.0f;
	offsetFromPlayer.z = cosf(m_rot.y) * cosf(m_rot.x) * m_fDistance;

	D3DXVECTOR3 idealCamPos = playerEye + offsetFromPlayer;

	// Bullet Physics でレイキャスト
	btVector3 from(playerEye.x, playerEye.y, playerEye.z);
	btVector3 to(idealCamPos.x, idealCamPos.y, idealCamPos.z);

	btCollisionWorld::ClosestRayResultCallback rayCallback(from, to);
	CManager::GetPhysicsWorld()->rayTest(from, to, rayCallback);

	if (rayCallback.hasHit())
	{
		// 衝突対象のユーザーポインタからブロックを特定
		const btCollisionObject* hitObj = rayCallback.m_collisionObject;
		void* userPtr = hitObj->getUserPointer();

		if (userPtr != nullptr)
		{
			// 衝突点の少し手前にカメラを配置
			btVector3 hitPoint = rayCallback.m_hitPointWorld;
			btVector3 camDir = (from - hitPoint).normalized();
			hitPoint += camDir * 10.0f;

			m_posV = D3DXVECTOR3(hitPoint.x(), hitPoint.y(), hitPoint.z());
		}
		else
		{// ユーザーポインタが null の場合

			// 補正しておく
			btVector3 hitPoint = rayCallback.m_hitPointWorld;
			btVector3 camDir = (from - hitPoint).normalized();
			hitPoint += camDir * 10.0f;

			m_posV = D3DXVECTOR3(hitPoint.x(), hitPoint.y(), hitPoint.z());
		}
	}
	else
	{// 衝突なし
		// 理想位置そのまま
		m_posV = idealCamPos;
	}

	// 注視点はプレイヤーの頭
	m_posR = playerEye;
}
//=============================================================================
// カメラのパラメータ設定処理
//=============================================================================
void CCamera::SetCamParameter(D3DXVECTOR3 posV, D3DXVECTOR3 posR, D3DXVECTOR3 rot, float fDistance)
{
	m_posV = posV;
	m_posR = posR;
	m_rot = rot;

	if (fDistance <= 0.0f)
	{
		m_fDistance = sqrtf(
			((m_posV.x - m_posR.x) * (m_posV.x - m_posR.x)) +
			((m_posV.y - m_posR.y) * (m_posV.y - m_posR.y)) +
			((m_posV.z - m_posR.z) * (m_posV.z - m_posR.z)));
	}
	else
	{
		m_fDistance = fDistance;
	}
}
//=============================================================================
// ムービー用カメラの設定処理
//=============================================================================
void CCamera::SetMovieCamera(const std::vector<CameraKeyFrame>& keys)
{
	m_camKeys = keys;
	m_currentKey = 0;
	m_directionFrame = 0;
	m_Mode = MODE_MOVIE;

	// 初期状態を1キー目に
	m_posV = keys[0].posV;
	m_posR = keys[0].posR;
	m_rot = keys[0].rot;

	if (keys[0].distance <= 0.0f)
	{
		m_fDistance = sqrtf(
			((m_posV.x - m_posR.x) * (m_posV.x - m_posR.x)) +
			((m_posV.y - m_posR.y) * (m_posV.y - m_posR.y)) +
			((m_posV.z - m_posR.z) * (m_posV.z - m_posR.z)));
	}
	else
	{
		m_fDistance = keys[0].distance;
	}
}
//=============================================================================
// 演出カメラの設定処理
//=============================================================================
void CCamera::SetCamDirection(int nTimer,D3DXVECTOR3 posV, D3DXVECTOR3 posR, D3DXVECTOR3 rot)
{
	m_Mode = MODE_DIRECTION;

	// タイマーの設定
	SetTimer(nTimer);

	m_posV = posV;
	m_posR = posR;
	m_vecU = D3DXVECTOR3(0.0f, 1.0f, 0.0f);// 固定でいい
	m_rot = rot;
	m_fDistance = sqrtf(
		((m_posV.x - m_posR.x) * (m_posV.x - m_posR.x)) +
		((m_posV.y - m_posR.y) * (m_posV.y - m_posR.y)) +
		((m_posV.z - m_posR.z) * (m_posV.z - m_posR.z)));
}
//=============================================================================
// 演出カメラの処理
//=============================================================================
void CCamera::DirectionCamera(int nTimer)
{
	m_nDirectionCamTimer++;

	if (m_nDirectionCamTimer >= nTimer)
	{
		m_isDirection = false;
		// カウンターリセット
		m_nDirectionCamTimer = 0;

		m_fDistance = 220.0f;

		// ゲームカメラに戻す
		m_Mode = MODE_GAME;
	}
}
//=============================================================================
// ムービーカメラの処理
//=============================================================================
void CCamera::MovieCamera(void)
{
	// 全キー終了
	if (m_currentKey >= (int)m_camKeys.size() - 1)
	{
		// 遅延フレームリセット
		m_delayFrame = 0;

		return;
	}

	CameraKeyFrame& from = m_camKeys[m_currentKey];
	CameraKeyFrame& to = m_camKeys[m_currentKey + 1];

	//==========================
	// 遅延処理
	//==========================
	if (m_delayFrame < from.delay)
	{
		m_delayFrame++;
		return; // まだ動かさない
	}

	int duration = std::max(1, to.duration); // ★0防止

	float t = (float)m_directionFrame / (float)duration;
	t = std::max(0.0f, std::min(1.0f, t));

	// 補間
	m_posV = from.posV + (to.posV - from.posV) * t;
	m_posR = from.posR + (to.posR - from.posR) * t;
	m_rot = from.rot + (to.rot - from.rot) * t;
	m_fDistance = from.distance + (to.distance - from.distance) * t;

	m_directionFrame++;

	if (m_directionFrame >= duration)
	{
		m_directionFrame = 0;
		m_currentKey++;
	}
}
//=============================================================================
// カメラの前方ベクトル取得
//=============================================================================
D3DXVECTOR3 CCamera::GetForward(void) const
{
	// カメラの回転角度（Y軸）から前方ベクトルを計算
	float yaw = m_rot.y;

	D3DXVECTOR3 forward(-sinf(yaw), 0.0f, -cosf(yaw));

	// 正規化する
	D3DXVec3Normalize(&forward, &forward);

	return forward;
}
//=============================================================================
// 角度の正規化 X軸
//=============================================================================
void CCamera::NormalizeRotX(void)
{
	if (m_rot.x > D3DX_PI * 0.5f)
	{
		m_rot.x = D3DX_PI * 0.5f;
	}
	if (m_rot.x < -D3DX_PI * 0.5f)
	{
		m_rot.x = -D3DX_PI * 0.5f;
	}
}
//=============================================================================
// 角度の正規化 Y軸
//=============================================================================
void CCamera::NormalizeRotY(void)
{
	if (m_rot.y > D3DX_PI)
	{
		m_rot.y -= D3DX_PI * 2.0f;
	}
	else if (m_rot.y < -D3DX_PI)
	{
		m_rot.y += D3DX_PI * 2.0f;
	}
}
//=============================================================================
// カメラシェイク処理
//=============================================================================
void CCamera::CameraShake(void)
{
	// --------------------------
	// 距離依存の常時振動
	// --------------------------
	if (m_isCameraShakeOn)
	{
		D3DXVECTOR3 targetpos = m_targetPos - m_posR;

		float dist = D3DXVec3Length(&targetpos);

		float maxDist = SHAKE_DISTANCE;// 振動する距離
		float ambientIntensity = 1.0f - (dist / maxDist);
		ambientIntensity = std::max(0.0f, std::min(1.0f, ambientIntensity));

		if (ambientIntensity > 0.0f)
		{
			static float t = 0.0f;
			t += 0.2f;
			m_shakeOffset.x += sinf(t * 8.0f) * 5.0f * ambientIntensity;
			m_shakeOffset.y += cosf(t * 12.0f) * 3.0f * ambientIntensity;
			m_shakeOffset.z += sinf(t * 6.0f) * 4.0f * ambientIntensity;
		}
	}

	// --------------------------
	// イベント依存の振動
	// --------------------------
	if (m_eventShakeTime > 0.0f)
	{
		float power = m_eventShakePower * (m_eventShakeTime / m_eventShakeDuration); // 減衰
		m_shakeOffset.x += ((rand() % 2000 - 1000) / 1000.0f) * power;
		m_shakeOffset.y += ((rand() % 2000 - 1000) / 1000.0f) * power;
		m_shakeOffset.z += ((rand() % 2000 - 1000) / 1000.0f) * power;

		m_eventShakeTime--; // 経過時間で減衰
	}

	if (m_Mode == MODE_DIRECTION || m_Mode == MODE_EDIT)
	{// 演出カメラだったら
		return;
	}

	// --------------------------
	// カメラ位置に反映
	// --------------------------
	m_posV += m_shakeOffset;
}
//=============================================================================
// イベント時のカメラシェイクの設定
//=============================================================================
void CCamera::TriggerEventShake(float power, float duration)
{
	m_eventShakePower = power;
	m_eventShakeDuration = duration;
	m_eventShakeTime = duration;
}
