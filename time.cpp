//=============================================================================
//
// タイム処理 [time.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "time.h"
#include "renderer.h"
#include "manager.h"

//=============================================================================
// コンストラクタ
//=============================================================================
CTime::CTime(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	memset(m_apNumber, 0, sizeof(m_apNumber));	// 各桁の数字表示用
	m_nMinutes		= 0;						// 分
	m_nSeconds		= 0;						// 秒
	m_nFrameCount	= 0;						// フレームカウント
	m_digitWidth	= 0.0f;						// 数字1桁あたりの幅
	m_digitHeight	= 0.0f;						// 数字1桁あたりの高さ
	m_basePos		= INIT_VEC3;				// 表示の開始位置
	m_pColon		= nullptr;					// コロン
	m_nIdxTexture	= 0;						// テクスチャインデックス
	m_nStartMinutes = 0;						// 経過時間の割合の結果代入用
	m_nStartSeconds = 0;						// 経過時間の割合の結果代入用
	m_isTimeUp		= false;					// タイムアップフラグ
	m_isActive		= false;					// アクティブフラグ
	m_isVisible		= true;						// 表示フラグ
}
//=============================================================================
// デストラクタ
//=============================================================================
CTime::~CTime()
{
	// なし
}
//=============================================================================
// 生成処理
//=============================================================================
CTime* CTime::Create(int minutes, int seconds,float baseX,float baseY,float digitWidth,float digitHeight, bool visibleFlag)
{
	CTime* pTime = new CTime;

	// nullptrだったら
	if (pTime == nullptr)
	{
		return nullptr;
	}

	pTime->m_nMinutes = minutes;
	pTime->m_nSeconds = seconds;
	pTime->m_nFrameCount = 0;
	pTime->m_basePos = D3DXVECTOR3(baseX, baseY, 0.0f);
	pTime->m_digitWidth = digitWidth;
	pTime->m_digitHeight = digitHeight;
	pTime->m_isVisible = visibleFlag;

	// 経過時間の割合を求めるために変数に代入
	pTime->m_nStartMinutes = pTime->m_nMinutes;
	pTime->m_nStartSeconds = pTime->m_nSeconds;

	// 初期化失敗時
	if (FAILED(pTime->Init()))
	{
		return nullptr;
	}

	return pTime;
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CTime::Init(void)
{
	// 分の数字（0,1桁）
	for (int nCnt = 0; nCnt < 2; nCnt++)
	{
		float posX = m_basePos.x + nCnt * m_digitWidth;
		float posY = m_basePos.y;

		// ナンバーの生成
		m_apNumber[nCnt] = CNumber::Create(posX, posY, m_digitWidth, m_digitHeight);

		if (!m_apNumber[nCnt])
		{
			return E_FAIL;
		}
	}

	// コロンの生成
	CColon::Create(D3DXVECTOR3(m_basePos.x + 2 * m_digitWidth, m_basePos.y, 0.0f), m_digitWidth / 2, m_digitHeight, m_isVisible);

	// コロンの幅
	float colonWidth = m_digitWidth / 2;

	// 秒の数字はコロンの幅だけ右にずらす
	for (int nCnt = 2; nCnt < DIGITS; nCnt++)
	{
		float posX = m_basePos.x + colonWidth + nCnt * m_digitWidth;
		float posY = m_basePos.y;

		m_apNumber[nCnt] = CNumber::Create(posX, posY, m_digitWidth, m_digitHeight);

		if (!m_apNumber[nCnt])
		{
			return E_FAIL;
		}
	}

	// テクスチャ割り当て
	CTexture* pTexture = CManager::GetTexture();
	m_nIdxTexture = pTexture->RegisterDynamic("data/TEXTURE/num_01.png");

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CTime::Uninit(void)
{
	for (int nCnt = 0; nCnt < DIGITS; nCnt++)
	{
		if (m_apNumber[nCnt] != nullptr)
		{
			// ナンバーの終了処理
			m_apNumber[nCnt]->Uninit();

			delete m_apNumber[nCnt];
			m_apNumber[nCnt] = nullptr;
		}
	}

	// オブジェクトの破棄(自分自身)
	this->Release();
}
//=============================================================================
// 更新処理
//=============================================================================
void CTime::Update(void)
{
	if (m_isActive)
	{
		// タイマーカウントダウン処理
		Countdown();
	}

	// 各桁の表示値を計算
	int min10 = m_nMinutes / 10;
	int min1 = m_nMinutes % 10;
	int sec10 = m_nSeconds / 10;
	int sec1 = m_nSeconds % 10;

	// ナンバーオブジェクトに反映
	if (m_apNumber[0]) m_apNumber[0]->SetDigit(min10);
	if (m_apNumber[1]) m_apNumber[1]->SetDigit(min1);
	if (m_apNumber[2]) m_apNumber[2]->SetDigit(sec10);
	if (m_apNumber[3]) m_apNumber[3]->SetDigit(sec1);

	for (int nCnt = 0; nCnt < DIGITS; nCnt++)
	{
		m_apNumber[nCnt]->Update();  // UV更新
	}
}
//=============================================================================
// タイマーカウントアップ処理
//=============================================================================
void CTime::Countup(void)
{
	// フレームカウント
	m_nFrameCount++;

	// 60フレーム経過したら1秒加算
	if (m_nFrameCount >= 60)
	{
		m_nFrameCount = 0;

		m_nSeconds++;

		// 60秒で1分繰り上げ
		if (m_nSeconds >= 60)
		{
			m_nSeconds = 0;
			m_nMinutes++;
		}
	}
}
//=============================================================================
// タイマーカウントダウン処理
//=============================================================================
void CTime::Countdown(void)
{
	// フレームカウント
	m_nFrameCount++;

	// 60フレーム経過したら1秒加算
	if (m_nFrameCount >= 60)
	{
		m_nFrameCount = 0;

		// 秒を1減らす
		if (m_nSeconds > 0)
		{
			m_nSeconds--;
		}
		else
		{
			// 秒が0の場合、分を1減らして秒を59にする
			if (m_nMinutes > 0)
			{
				m_nMinutes--;
				m_nSeconds = 59;
			}
			else
			{
				// 分も0ならタイムアップ
				m_nSeconds = 0;

				m_isTimeUp = true;
			}
		}
	}
}
//=============================================================================
// 描画処理
//=============================================================================
void CTime::Draw(void)
{
	// テクスチャの取得
	CTexture* pTexture = CManager::GetTexture();

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// 表示OFFの場合はスキップ
	if (!m_isVisible)
	{
		return;
	}

	for (int nCnt = 0; nCnt < DIGITS; nCnt++)
	{
		if (m_apNumber[nCnt])
		{
			// テクスチャの設定
			pDevice->SetTexture(0, pTexture->GetAddress(m_nIdxTexture));

			// ナンバーの描画処理
			m_apNumber[nCnt]->Draw();
		}
	}
}
//=============================================================================
// 経過割合を取得する関数（0.0f ～ 1.0f）
//=============================================================================
float CTime::GetProgress(void) const
{
	// 開始時の総フレーム数
	int totalFrames = (m_nStartMinutes * 60 + m_nStartSeconds) * 60;

	// 現在の残りフレーム数
	int currentFrames = (m_nMinutes * 60 + m_nSeconds) * 60 + (60 - m_nFrameCount);

	// 経過率を計算（0.0 = 開始、1.0 = 終了）
	float progress = 1.0f - (float)currentFrames / (float)totalFrames;

	// 範囲を制限
	if (progress < 0.0f) progress = 0.0f;
	if (progress > 1.0f) progress = 1.0f;

	return progress;
}


//=============================================================================
// コロンのコンストラクタ
//=============================================================================
CColon::CColon(int nPriority) : CObject(nPriority)
{
	// 値のクリア
	m_pVtxBuff		= nullptr;		// 頂点バッファ
	m_pos			= INIT_VEC3;	// 位置
	m_fWidth		= 0.0f;			// 幅
	m_fHeight		= 0.0f;			// 高さ
	m_nIdxTexture	= 0;			// テクスチャインデックス
	m_isVisible		= true;			// 表示フラグ
}
//=============================================================================
// コロンのデストラクタ
//=============================================================================
CColon::~CColon()
{
	// なし
}
//=============================================================================
// コロンの生成処理
//=============================================================================
CColon* CColon::Create(D3DXVECTOR3 pos, float fWidth, float fHeight, bool visibleFlag)
{
	CColon* pColon = new CColon;

	// nullptrだったら
	if (pColon == nullptr)
	{
		return nullptr;
	}

	pColon->m_pos = pos;
	pColon->m_fWidth = fWidth;
	pColon->m_fHeight = fHeight;
	pColon->SetVisible(visibleFlag);

	// 初期化失敗時
	if (FAILED(pColon->Init()))
	{
		return nullptr;
	}

	return pColon;
}
//=============================================================================
// コロンの初期化処理
//=============================================================================
HRESULT CColon::Init(void)
{
	// デバイス取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// テクスチャの登録
	m_nIdxTexture = CManager::GetTexture()->RegisterDynamic("data/TEXTURE/colon.png");

	// 頂点バッファの生成
	pDevice->CreateVertexBuffer(sizeof(VERTEX_2D) * 4,
		D3DUSAGE_WRITEONLY,
		FVF_VERTEX_2D,
		D3DPOOL_MANAGED,
		&m_pVtxBuff,
		NULL);

	VERTEX_2D* pVtx;// 頂点情報へのポインタ

	// 頂点バッファをロックし、頂点情報へのポインタを取得
	m_pVtxBuff->Lock(0, 0, (void**)&pVtx, 0);

	// 頂点座標の設定
	pVtx[0].pos = D3DXVECTOR3(m_pos.x, m_pos.y, 0.0f);
	pVtx[1].pos = D3DXVECTOR3(m_pos.x + m_fWidth, m_pos.y, 0.0f);
	pVtx[2].pos = D3DXVECTOR3(m_pos.x, m_pos.y + m_fHeight, 0.0f);
	pVtx[3].pos = D3DXVECTOR3(m_pos.x + m_fWidth, m_pos.y + m_fHeight, 0.0f);

	// rhwの設定
	pVtx[0].rhw = 1.0f;
	pVtx[1].rhw = 1.0f;
	pVtx[2].rhw = 1.0f;
	pVtx[3].rhw = 1.0f;

	// 頂点カラーの設定
	pVtx[0].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVtx[1].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVtx[2].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
	pVtx[3].col = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	// テクスチャ座標の設定
	pVtx[0].tex = D3DXVECTOR2(0.0f, 0.0f);
	pVtx[1].tex = D3DXVECTOR2(1.0f, 0.0f);
	pVtx[2].tex = D3DXVECTOR2(0.0f, 1.0f);
	pVtx[3].tex = D3DXVECTOR2(1.0f, 1.0f);

	// 頂点バッファをアンロックする
	m_pVtxBuff->Unlock();

	return S_OK;
}
//=============================================================================
// コロンの終了処理
//=============================================================================
void CColon::Uninit(void)
{
	// 頂点バッファの破棄
	if (m_pVtxBuff != nullptr)
	{
		m_pVtxBuff->Release();
		m_pVtxBuff = nullptr;
	}

	CObject::Release();
}
//=============================================================================
// コロンの更新処理
//=============================================================================
void CColon::Update(void)
{




}
//=============================================================================
// コロンの描画処理
//=============================================================================
void CColon::Draw(void)
{
	// テクスチャの取得
	CTexture* pTexture = CManager::GetTexture();

	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	// 表示OFFの場合はスキップ
	if (!m_isVisible)
	{
		return;
	}

	// 頂点バッファをデータストリームに設定
	pDevice->SetStreamSource(0, m_pVtxBuff, 0, sizeof(VERTEX_2D));

	// 頂点フォーマットの設定
	pDevice->SetFVF(FVF_VERTEX_2D);

	// テクスチャの設定
	pDevice->SetTexture(0, pTexture->GetAddress(m_nIdxTexture));

	// ポリゴンの描画
	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
}
