//=============================================================================
//
// サウンド処理 [sound.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "sound.h"
#include "game.h"
#include "manager.h"

// 名前空間stdの使用
using namespace std;

//=============================================================================
// コンストラクタ
//=============================================================================
CSound::CSound()
{
	// 値のクリア
	m_pXAudio2			= nullptr;
	m_pMasteringVoice	= nullptr;
	m_Listener			= {};					// リスナーの位置
	m_nextInstanceId	= 0;
}
//=============================================================================
// デストラクタ
//=============================================================================
CSound::~CSound()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CSound::Init(HWND hWnd)
{
	HRESULT hr;

	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	hr = XAudio2Create(&m_pXAudio2, 0);
	if (FAILED(hr))
	{
		MessageBox(hWnd, "XAudio2オブジェクトの作成に失敗！", "警告！", MB_ICONWARNING);
		CoUninitialize();
		return hr;
	}

	XAUDIO2_DEVICE_DETAILS deviceDetails;
	hr = m_pXAudio2->GetDeviceDetails(0, &deviceDetails);
	if (FAILED(hr))
	{
		MessageBox(hWnd, "デバイス情報の取得に失敗！", "警告！", MB_ICONWARNING);
		m_pXAudio2->Release();
		m_pXAudio2 = nullptr;
		CoUninitialize();
		return hr;
	}

	UINT32 channels = deviceDetails.OutputFormat.Format.nChannels;
	UINT32 sampleRate = deviceDetails.OutputFormat.Format.nSamplesPerSec;

	hr = m_pXAudio2->CreateMasteringVoice(&m_pMasteringVoice, channels, sampleRate);
	if (FAILED(hr))
	{
		MessageBox(hWnd, "マスターボイスの生成に失敗！", "警告！", MB_ICONWARNING);
		m_pXAudio2->Release();
		m_pXAudio2 = nullptr;
		CoUninitialize();
		return hr;
	}

	X3DAudioInitialize(deviceDetails.OutputFormat.dwChannelMask, X3DAUDIO_SPEED_OF_SOUND, m_X3DInstance);

	m_Listener.Position = { 0.0f, 0.0f, 0.0f };
	m_Listener.OrientFront = { 0.0f, 0.0f, 1.0f };
	m_Listener.OrientTop = { 0.0f, 1.0f, 0.0f };
	m_Listener.Velocity = { 0.0f, 0.0f, 0.0f };

	for (int i = 0; i < SOUND_LABEL_MAX; ++i)
	{
		hr = LoadWave((SOUND_LABEL)i);
		if (FAILED(hr))
		{
			char msg[128];
			sprintf_s(msg, "サウンドの読み込みに失敗: %s", m_aSoundInfo[i].pFilename);
			MessageBox(hWnd, msg, "Error", MB_OK);
			Uninit();
			return hr;
		}
	}
	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CSound::Uninit(void)
{
	// 全てのSoundInstanceを停止・破棄
	for (auto& inst : m_Instances)
	{
		if (inst.pSourceVoice)
		{
			inst.pSourceVoice->Stop(0);
			inst.pSourceVoice->DestroyVoice();
			inst.pSourceVoice = nullptr;
		}
	}
	m_Instances.clear();

	// 各サウンドのオーディオデータ解放
	for (int i = 0; i < SOUND_LABEL_MAX; ++i)
	{
		if (m_SoundData[i].pAudioData)
		{
			free(m_SoundData[i].pAudioData);
			m_SoundData[i].pAudioData = nullptr;
		}
	}

	// マスターボイスの破棄
	if (m_pMasteringVoice)
	{
		m_pMasteringVoice->DestroyVoice();
		m_pMasteringVoice = nullptr;
	}

	// XAudio2オブジェクトの開放
	if (m_pXAudio2)
	{
		m_pXAudio2->Release();
		m_pXAudio2 = nullptr;
	}

	// COMライブラリの終了処理
	CoUninitialize();
}
//=============================================================================
// 2Dセグメント再生(再生中なら停止)
//=============================================================================
int CSound::Play(SOUND_LABEL label)
{
	if (label < 0 || label >= SOUND_LABEL_MAX)
	{
		return -1;
	}

	SoundInstance inst = {};
	inst.id = m_nextInstanceId++;
	inst.label = label;
	inst.active = true;

	HRESULT hr = m_pXAudio2->CreateSourceVoice(&inst.pSourceVoice, &(m_SoundData[label].wfx.Format));
	if (FAILED(hr))
	{
		return -1;
	}

	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = m_SoundData[label].audioBytes;
	buffer.pAudioData = m_SoundData[label].pAudioData;
	if (m_aSoundInfo[label].loopCount == XAUDIO2_LOOP_INFINITE)
	{// ループの時
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}
	else
	{// ループじゃない時
		buffer.LoopCount = m_aSoundInfo[label].loopCount;
		buffer.Flags = XAUDIO2_END_OF_STREAM;
	}

	hr = inst.pSourceVoice->SubmitSourceBuffer(&buffer);
	if (FAILED(hr))
	{
		inst.pSourceVoice->DestroyVoice();
		return -1;
	}

	hr = inst.pSourceVoice->Start(0);
	if (FAILED(hr))
	{
		inst.pSourceVoice->DestroyVoice();
		return -1;
	}

	m_Instances.push_back(inst);

	return inst.id;
}
//=============================================================================
// 3Dセグメント再生(再生中なら停止)
//=============================================================================
int CSound::Play3D(SOUND_LABEL label, D3DXVECTOR3 soundPos, float minDistance, float maxDistance)
{
	if (label < 0 || label >= SOUND_LABEL_MAX)
	{
		return -1;
	}

	SoundInstance inst = {};
	inst.id = m_nextInstanceId++;
	inst.label = label;
	inst.active = true;

	HRESULT hr = m_pXAudio2->CreateSourceVoice(&inst.pSourceVoice, &(m_SoundData[label].wfx.Format));
	if (FAILED(hr))
	{
		return -1;
	}

	// エミッター設定
	inst.emitter.Position = { soundPos.x, soundPos.y, soundPos.z };
	inst.emitter.Velocity = { 0.0f, 0.0f, 0.0f };
	inst.emitter.ChannelCount = 1;
	inst.emitter.CurveDistanceScaler = 150.0f;

	// 距離範囲を保存
	inst.minDistance = minDistance;
	inst.maxDistance = maxDistance;

	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = m_SoundData[label].audioBytes;
	buffer.pAudioData = m_SoundData[label].pAudioData;

	if (m_aSoundInfo[label].loopCount == XAUDIO2_LOOP_INFINITE)
	{// ループの時
		buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
	}
	else
	{// ループじゃない時
		buffer.LoopCount = m_aSoundInfo[label].loopCount;
		buffer.Flags = XAUDIO2_END_OF_STREAM;
	}

	hr = inst.pSourceVoice->SubmitSourceBuffer(&buffer);
	if (FAILED(hr))
	{
		inst.pSourceVoice->DestroyVoice();
		return -1;
	}

	// 3Dパンニング行列を計算
	FLOAT32 matrix[8] = {}; // 適切なサイズ（出力チャンネル×入力チャンネル）
	CalculateCustomPanning(inst, matrix);

	// パンニング適用（出力チャンネル4、入力1）
	inst.pSourceVoice->SetOutputMatrix(NULL, 1, 4, matrix);

	hr = inst.pSourceVoice->Start(0);
	if (FAILED(hr))
	{
		inst.pSourceVoice->DestroyVoice();
		return -1;
	}

	m_Instances.push_back(inst);

	return inst.id;
}
//=============================================================================
// 一時停止
//=============================================================================
void CSound::PauseAll(void)
{
	for (auto& inst : m_Instances)
	{
		if (inst.pSourceVoice)
		{
			inst.pSourceVoice->Stop(0); // フラグ0で一時停止
		}
	}
}
//=============================================================================
// 再開
//=============================================================================
void CSound::ResumeAll(void)
{
	for (auto& inst : m_Instances)
	{
		if (inst.pSourceVoice)
		{
			inst.pSourceVoice->Start(0); // フラグ0で再開
		}
	}
}
//=============================================================================
// セグメント停止(インデックス指定)
//=============================================================================
void CSound::Stop(int instanceId)
{
	for (auto it = m_Instances.begin(); it != m_Instances.end(); it++)
	{
		if (it->id == instanceId)
		{
			if (it->pSourceVoice)
			{
				it->pSourceVoice->Stop(0);
				it->pSourceVoice->FlushSourceBuffers();
				it->pSourceVoice->DestroyVoice();
				it->pSourceVoice = nullptr;
			}
			m_Instances.erase(it);
			return;
		}
	}
}
//=============================================================================
// セグメント停止(ラベル指定)
//=============================================================================
void CSound::StopByLabel(SOUND_LABEL label)
{
	for (auto it = m_Instances.begin(); it != m_Instances.end(); )
	{
		if (it->label == label)
		{
			if (it->pSourceVoice)
			{
				it->pSourceVoice->Stop(0);
				it->pSourceVoice->FlushSourceBuffers();
				it->pSourceVoice->DestroyVoice();
			}
			it = m_Instances.erase(it);
		}
		else
		{
			it++;
		}
	}
}
//=============================================================================
// セグメント停止(全て)
//=============================================================================
void CSound::Stop(void)
{
	for (auto& inst : m_Instances)
	{
		if (inst.pSourceVoice)
		{
			inst.pSourceVoice->Stop(0);
			inst.pSourceVoice->FlushSourceBuffers();
			inst.pSourceVoice->DestroyVoice();
		}
	}
	m_Instances.clear();  // 一括でクリア
}
//=============================================================================
// カスタムパンニング
//=============================================================================
void CSound::CalculateCustomPanning(SoundInstance& inst, FLOAT32* matrix)
{
	// プレイヤーの向き（前方向ベクトル）
	D3DXVECTOR3 front = m_Listener.OrientFront;
	D3DXVec3Normalize(&front, &front);

	// 上方向ベクトル（通常は (0, 1, 0)）
	D3DXVECTOR3 up = m_Listener.OrientTop;
	D3DXVec3Normalize(&up, &up);

	// 右方向ベクトル = front × up（外積で求める）
	D3DXVECTOR3 right;
	D3DXVec3Cross(&right, &up, &front);
	D3DXVec3Normalize(&right, &right);

	// リスナー → 音源のベクトル
	D3DXVECTOR3 toEmitter =
	{
		inst.emitter.Position.x - m_Listener.Position.x,
		inst.emitter.Position.y - m_Listener.Position.y,
		inst.emitter.Position.z - m_Listener.Position.z
	};

	// 距離を計算
	float distance = D3DXVec3Length(&toEmitter);

	// 距離減衰計算
	float volumeScale = 1.0f - ((distance - inst.minDistance) / (inst.maxDistance - inst.minDistance));
	volumeScale = max(0.0f, min(1.0f, volumeScale));

	// 方向ベクトルを正規化
	D3DXVec3Normalize(&toEmitter, &toEmitter);

	// 左右・前後パンニング因子
	float panLR = D3DXVec3Dot(&right, &toEmitter); // -1 (左) ～ 1 (右)
	float panFB = D3DXVec3Dot(&front, &toEmitter); // -1 (後) ～ 1 (前)

	// 方向補正
	float lrWeight = panLR * 0.5f; // -0.5 ～ +0.5
	float fbWeight = panFB * 0.5f + 0.5f; // 0（背後）～1（正面）

	// 背後でも最低限音が残るように補正
	const float minVolume = 0.3f;
	fbWeight = fbWeight * (1.0f - minVolume) + minVolume;
	float backWeight = 1.0f - fbWeight;

	// 音量設定
	float frontLeft = (0.5f - lrWeight) * fbWeight * volumeScale;
	float frontRight = (0.5f + lrWeight) * fbWeight * volumeScale;
	float rearLeft = (0.5f - lrWeight) * backWeight * volumeScale;
	float rearRight = (0.5f + lrWeight) * backWeight * volumeScale;

	// 音量マトリックスにセット（4ch: FL, FR, RL, RR）
	matrix[0] = frontLeft;
	matrix[1] = frontRight;
	matrix[2] = rearLeft;
	matrix[3] = rearRight;
}
//=============================================================================
// リスナー(カメラ)の更新処理
//=============================================================================
void CSound::UpdateListener(D3DXVECTOR3 pos)
{
	// カメラの向きベクトルを計算
	D3DXVECTOR3 forward = CManager::GetCamera()->GetForward();

	// 正規化して代入
	D3DXVec3Normalize(&forward, &forward);

	m_Listener.OrientFront = forward;

	D3DXVECTOR3 orienttop;
	orienttop.x = 0.0f;
	orienttop.y = 1.0f;
	orienttop.z = 0.0f;

	D3DXVec3Normalize(&orienttop, &orienttop);

	m_Listener.OrientTop = orienttop;

	m_Listener.Position = { pos.x, pos.y, pos.z };				// リスナー(カメラ)の位置
}
//=============================================================================
// 音源の位置更新処理
//=============================================================================
void CSound::UpdateSoundPosition(int instanceId, D3DXVECTOR3 pos)
{
	for (auto& inst : m_Instances)
	{
		if (inst.id == instanceId)
		{
			// 音源の現在位置を適用
			inst.emitter.Position = { pos.x, pos.y, pos.z };

			// 3Dオーディオ計算
			X3DAUDIO_DSP_SETTINGS dspSettings = {};
			FLOAT32 matrix[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			dspSettings.SrcChannelCount = 1;
			dspSettings.DstChannelCount = 4;
			dspSettings.pMatrixCoefficients = matrix;

			X3DAudioCalculate(
				m_X3DInstance,
				&m_Listener,
				&inst.emitter,
				X3DAUDIO_CALCULATE_MATRIX,
				&dspSettings
			);

			// パンニング値をクリップ
			for (int nCnt = 0; nCnt < 4; nCnt++)
			{
				matrix[nCnt] = max(0.0f, min(1.0f, matrix[nCnt]));
			}

			// カスタムパンニング計算
			CalculateCustomPanning(inst, matrix);

			// 出力マトリクスを更新
			if (inst.pSourceVoice)
			{
				inst.pSourceVoice->SetOutputMatrix(NULL, 1, 4, matrix);
			}
			break;
		}
	}
}
//=============================================================================
// チャンクのチェック
//=============================================================================
HRESULT CSound::CheckChunk(HANDLE hFile, DWORD format, DWORD* pChunkSize, DWORD* pChunkDataPosition)
{
	HRESULT hr = S_OK;
	DWORD dwRead;
	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD dwBytesRead = 0;
	DWORD dwOffset = 0;

	if (SetFilePointer(hFile, 0, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{// ファイルポインタを先頭に移動
		return HRESULT_FROM_WIN32(GetLastError());
	}

	while (hr == S_OK)
	{
		if (ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL) == 0)
		{// チャンクの読み込み
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		if (ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL) == 0)
		{// チャンクデータの読み込み
			hr = HRESULT_FROM_WIN32(GetLastError());
		}

		switch (dwChunkType)
		{
		case 'FFIR':
			dwRIFFDataSize = dwChunkDataSize;
			dwChunkDataSize = 4;
			if (ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL) == 0)
			{// ファイルタイプの読み込み
				hr = HRESULT_FROM_WIN32(GetLastError());
			}
			break;

		default:
			if (SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			{// ファイルポインタをチャンクデータ分移動
				return HRESULT_FROM_WIN32(GetLastError());
			}
		}

		dwOffset += sizeof(DWORD) * 2;
		if (dwChunkType == format)
		{
			*pChunkSize = dwChunkDataSize;
			*pChunkDataPosition = dwOffset;

			return S_OK;
		}

		dwOffset += dwChunkDataSize;
		if (dwBytesRead >= dwRIFFDataSize)
		{
			return S_FALSE;
		}
	}

	return S_OK;
}
//=============================================================================
// チャンクデータの読み込み
//=============================================================================
HRESULT CSound::ReadChunkData(HANDLE hFile, void* pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset)
{
	DWORD dwRead;

	if (SetFilePointer(hFile, dwBufferoffset, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
	{// ファイルポインタを指定位置まで移動
		return HRESULT_FROM_WIN32(GetLastError());
	}

	if (ReadFile(hFile, pBuffer, dwBuffersize, &dwRead, NULL) == 0)
	{// データの読み込み
		return HRESULT_FROM_WIN32(GetLastError());
	}

	return S_OK;
}
//=============================================================================
// WAVEの読み込み
//=============================================================================
HRESULT CSound::LoadWave(SOUND_LABEL label)
{
	if (label < 0 || label >= SOUND_LABEL_MAX)
	{
		return E_FAIL;
	}

	HANDLE hFile = CreateFileA(
		m_aSoundInfo[label].pFilename,
		GENERIC_READ, FILE_SHARE_READ,
		NULL, OPEN_EXISTING,
		0, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
		return E_FAIL;

	DWORD dwChunkSize;
	DWORD dwChunkPosition;

	// "RIFF" チャンクチェック
	if (FAILED(CheckChunk(hFile, 'FFIR', &dwChunkSize, &dwChunkPosition)))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// "fmt " チャンク
	if (FAILED(CheckChunk(hFile, ' tmf', &dwChunkSize, &dwChunkPosition)))
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	WAVEFORMATEXTENSIBLE wfx = {};
	if (FAILED(ReadChunkData(hFile, &wfx, dwChunkSize, dwChunkPosition))) 
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	// "data" チャンク
	if (FAILED(CheckChunk(hFile, 'atad', &dwChunkSize, &dwChunkPosition))) 
	{
		CloseHandle(hFile);
		return E_FAIL;
	}

	BYTE* pDataBuffer = new BYTE[dwChunkSize];
	if (FAILED(ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition))) 
	{
		delete[] pDataBuffer;
		CloseHandle(hFile);
		return E_FAIL;
	}

	// ソースボイス作成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	if (FAILED(m_pXAudio2->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)&wfx)))
	{
		delete[] pDataBuffer;
		CloseHandle(hFile);
		return E_FAIL;
	}

	// バッファ設定
	XAUDIO2_BUFFER buffer = {};
	buffer.AudioBytes = dwChunkSize;
	buffer.pAudioData = pDataBuffer;
	buffer.Flags = XAUDIO2_END_OF_STREAM;

	pSourceVoice->SubmitSourceBuffer(&buffer);

	// 保持
	m_SoundData[label].pAudioData = pDataBuffer;	// 読み込んだPCMデータのバッファ
	m_SoundData[label].audioBytes = dwChunkSize;	// データのバイト数
	m_SoundData[label].wfx = wfx;					// フォーマット情報

	CloseHandle(hFile);
	return S_OK;
}
