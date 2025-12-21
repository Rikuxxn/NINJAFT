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
		SOUND_LABEL_TITLEBGM,
		SOUND_LABEL_TUTORIALBGM,
		SOUND_LABEL_RESULTSE,
		SOUND_LABEL_PAUSE,
		SOUND_LABEL_SELECT,
		SOUND_LABEL_ENTER,
		SOUND_LABEL_GRASS,
		SOUND_LABEL_WATER,
		SOUND_LABEL_START,
		SOUND_LABEL_ITEMGET,
		SOUND_LABEL_STEP,
		SOUND_LABEL_SLASH_1,
		SOUND_LABEL_SLASH_2,
		SOUND_LABEL_DAMAGE,
		SOUND_LABEL_GATE_OPEN,
		SOUND_LABEL_GEAR,

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
		{"data/BGM/gameBGM.wav", -1},			// ゲームBGM
		{"data/BGM/titleBGM.wav", -1},			// タイトルBGM
		{"data/BGM/wind.wav", -1},				// チュートリアルBGM
		{"data/SE/resultSE.wav", 0},			// リザルトSE
		{"data/SE/menu.wav", 0},				// ポーズSE
		{"data/SE/select.wav", 0},				// 選択SE
		{"data/SE/enter.wav", 0},				// 決定SE
		{"data/SE/grass.wav", 0},				// 草SE
		{"data/SE/water.wav", 0},				// 水SE
		{"data/SE/start.wav", 0},				// 開始SE
		{"data/SE/itemget.wav", 0},				// アイテム取得SE
		{"data/SE/step.wav", 0},				// 足音SE
		{"data/SE/slash_1.wav", 0},				// 斬撃1SE
		{"data/SE/slash_2.wav", 0},				// 斬撃2SE
		{"data/SE/damage.wav", 0},				// ダメージSE
		{"data/SE/gate_open.wav", 0},			// 開門SE
		{"data/SE/gear.wav", -1},				// ギアSE(ループ)
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