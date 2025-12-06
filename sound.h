//=============================================================================
//
// サウンド処理 [sound.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _SOUND_H_// このマクロ定義がされていなかったら
#define _SOUND_H_// 2重インクルード防止のマクロ定義

//*************************************************************************
// サウンドクラス
//*************************************************************************
class CSound
{
public:
	CSound();
	~CSound();

	//*************************************************************************
	// サウンド一覧
	//*************************************************************************
	typedef enum
	{
		SOUND_LABEL_GAMEBGM = 0,	// ゲームBGM
		SOUND_LABEL_WATER,
		SOUND_LABEL_WATERRISE,
		SOUND_LABEL_HIT,
		SOUND_LABEL_FIRE,
		SOUND_LABEL_ROCKHIT,
		SOUND_LABEL_SWITCH,
		SOUND_LABEL_PAUSE,
		SOUND_LABEL_SELECT,
		SOUND_LABEL_ENTER,
		SOUND_LABEL_ROLL,
		SOUND_LABEL_MASK,
		SOUND_LABEL_INSPIRATION,
		SOUND_LABEL_TIMER,
		SOUND_LABEL_TREASURE,
		SOUND_LABEL_SWING,
		SOUND_LABEL_GET,
		SOUND_LABEL_RANK,
		SOUND_LABEL_GAMEBGM2,
		SOUND_LABEL_SHINE,
		SOUND_LABEL_DOOR,
		SOUND_LABEL_WATERWHEEL,
		SOUND_LABEL_WATERFLOW,
		SOUND_LABEL_FLOATING,
		SOUND_LABEL_RELEASE,
		SOUND_LABEL_MAX,
	} SOUND_LABEL;

	HRESULT Init(HWND hWnd);
	void Uninit(void);

	void PauseAll(void);
	void ResumeAll(void);

	void Stop(int instanceId);			// インデックス指定での停止(3Dサウンド)
	void StopByLabel(SOUND_LABEL label);// ラベル指定での停止(2Dサウンド)
	void Stop(void);
	int Play(SOUND_LABEL label);
	int Play3D(SOUND_LABEL label,D3DXVECTOR3 soundPos,float minDistance,float maxDistance);

	void UpdateListener(D3DXVECTOR3 pos);
	void UpdateSoundPosition(int instanceId, D3DXVECTOR3 pos);

private:
	// 最大同時再生数
	static constexpr int MAX_SIMULTANEOUS_PLAY = 2;

	// 一つのサウンド再生インスタンス
	struct SoundInstance
	{
		int id = -1;
		IXAudio2SourceVoice* pSourceVoice = nullptr;
		X3DAUDIO_EMITTER emitter = {};
		SOUND_LABEL label;
		bool active = false;
		float minDistance = 0.0f, maxDistance = 0.0f;						// 最小距離と最大距離（距離減衰の範囲）
	};

	struct SoundData
	{
		BYTE* pAudioData = nullptr;
		DWORD audioBytes = 0;
		WAVEFORMATEXTENSIBLE wfx = {};
	};

	struct SOUNDINFO
	{
		const char* pFilename;
		int loopCount;
	};

private:
	IXAudio2* m_pXAudio2;									// XAudio2オブジェクトへのインターフェイス
	IXAudio2MasteringVoice* m_pMasteringVoice;				// マスターボイス

	// サウンドの情報
	SOUNDINFO m_aSoundInfo[SOUND_LABEL_MAX] =
	{
		{"data/BGM/gameBGM.wav", -1},			// ゲームBGM(ステージ1)
		{"data/SE/water.wav", 0},				// 入水SE
		{"data/SE/waterrise.wav", 0},			// 水位上昇SE
		{"data/SE/hit.wav", 0},					// プレイヤーヒットSE
		{"data/SE/fire.wav", -1},				// 炎SE(ループ)
		{"data/SE/rock_hit.wav", 0},			// 岩衝突SE
		{"data/SE/switch.wav", 0},				// スイッチSE
		{"data/SE/menu.wav", 0},				// ポーズSE
		{"data/SE/select.wav", 0},				// 選択SE
		{"data/SE/enter.wav", 0},				// 決定SE
		{"data/SE/roll.wav", -1},				// 岩転がるSE(ループ)
		{"data/SE/mask.wav", -1},				// 仮面SE(ループ)
		{"data/SE/Inspiration.wav", 0},			// ひらめきSE
		{"data/SE/timer.wav", 0},				// タイマーSE
		{"data/SE/treasure.wav", -1},			// 秘宝SE(ループ)
		{"data/SE/swing.wav", 0},				// 斧SE
		{"data/SE/get.wav", 0},					// 入手SE
		{"data/SE/resultrank.wav", 0},			// リザルトランク表示SE
		{"data/BGM/gameBGM2.wav", -1},			// ゲームBGM2(ステージ2)
		{"data/SE/shines.wav", 0},				// ドアが開くときの台座SE
		{"data/SE/door.wav", 0},				// ドアSE
		{"data/SE/waterWheel.wav", -1},			// 歯車SE(ループ)
		{"data/SE/waterrise2.wav", -1},			// 水流SE(ループ)
		{"data/SE/magic_floating.wav", -1},		// 浮遊SE(ループ)
		{"data/SE/magic_release.wav", 0},		// 離しSE
	};

	SoundData m_SoundData[SOUND_LABEL_MAX];

	// インスタンス管理
	std::vector<SoundInstance> m_Instances;
	int m_nextInstanceId;

	X3DAUDIO_HANDLE m_X3DInstance;							// X3DAudio インスタンス
	X3DAUDIO_LISTENER m_Listener;							// リスナー（プレイヤーの位置）

private:
	HRESULT LoadWave(SOUND_LABEL label);
	void CalculateCustomPanning(SoundInstance& inst, FLOAT32* matrix);
	HRESULT CheckChunk(HANDLE hFile, DWORD format, DWORD* pChunkSize, DWORD* pChunkDataPosition);
	HRESULT ReadChunkData(HANDLE hFile, void* pBuffer, DWORD dwBuffersize, DWORD dwBufferoffset);

};

#endif