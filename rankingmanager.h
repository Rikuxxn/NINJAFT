//=============================================================================
//
// ランキングマネージャー処理 [rankingmanager.h]
// Author : RIKU TANEKAWA
//
//=============================================================================
#ifndef _RANKINGMANAGER_H_// このマクロ定義がされていなかったら
#define _RANKINGMANAGER_H_// 2重インクルード防止のマクロ定義

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include <algorithm>

//*****************************************************************************
// ランキングマネージャークラス
//*****************************************************************************
class CRankingManager
{
private:
    // ランキングデータ構造体
    struct RankData
    {
        // クリアタイム
        int minutes = 0;
        int seconds = 0;

        // アイテム個数
        int items = 0;
    };

    static constexpr int MAX_RANK = 5;

    std::vector<RankData> m_rankList;
    static CRankingManager* m_Instance;
    int m_rankIdx;// 順位のインデックス

public:
	CRankingManager();
	~CRankingManager();

    enum class RankType
    {
        ClearTime,  // 速い順
        ItemCount   // 多い順
    };

    static CRankingManager* GetInstance(void)
    {
        m_Instance = new CRankingManager();

        return m_Instance;
    }

    // 外部からコピー用に取得
    std::vector<RankData> GetRankList(void) const
    {
        std::vector<RankData> list;

        for (const auto& r : m_rankList)
        {
            list.push_back({ r.minutes, r.seconds, r.items });
        }
        return list;
    }

    //*****************************************************************************
    // 通常のタイムをそのまま登録
    //*****************************************************************************
    void AddRecord(int minutes, int seconds)
    {
        RankData data = { minutes, seconds };
        RegisterRecord(data);
    }

    //*****************************************************************************
    // アイテムの数を記録
    //*****************************************************************************
    void AddItemRecord(int itemCount)
    {
        RankData data;
        data.items = itemCount;
        RegisterRecord(data);
    }

    //*****************************************************************************
    // 残り時間をクリアタイムに変換して登録
    //*****************************************************************************
    void AddRecordWithLimit(int limitMinutes, int limitSeconds, int remainMinutes, int remainSeconds)
    {
        RankData data = ConvertToClearTime(limitMinutes, limitSeconds, remainMinutes, remainSeconds);
        RegisterRecord(data);
    }

    void SetRankType(RankType type)
    {
        m_rankType = type;
        SortRank();   // 切り替えたら並び替え
    }

    const std::vector<RankData>& GetList(void) const { return m_rankList; }
    int GetRankIdx(void) { return m_rankIdx; }

    void Save(void);
    void Load(void);

private:
    RankType m_rankType = RankType::ItemCount;

    //*****************************************************************************
    // 残り時間 → クリアタイムへの変換
    //*****************************************************************************
    RankData ConvertToClearTime(int limitMinutes, int limitSeconds, int remainMinutes, int remainSeconds)
    {
        int totalLimit = limitMinutes * 60 + limitSeconds;
        int totalRemain = remainMinutes * 60 + remainSeconds;

        int totalClear = totalLimit - totalRemain;
        if (totalClear < 0)
        {
            totalClear = 0;
        }

        RankData data;
        data.minutes = totalClear / 60;
        data.seconds = totalClear % 60;
        return data;
    }

    //*****************************************************************************
    // ランクリストへの登録処理（共通）
    //*****************************************************************************
    void RegisterRecord(const RankData& data)
    {
        m_rankList.push_back(data);

        // ソート処理
        SortRank();

        // 上位5件に制限
        if (m_rankList.size() > MAX_RANK)
        {
            m_rankList.resize(MAX_RANK);
        }

        // 実際に何位に入ったかを探す（上位5件の中で）
        m_rankIdx = -1; // 初期値はランクイン無し
        for (size_t i = 0; i < m_rankList.size(); i++)
        {
            if ((m_rankList[i].minutes == data.minutes &&
                m_rankList[i].seconds == data.seconds) &&
                m_rankList[i].items == data.items)
            {
                m_rankIdx = static_cast<int>(i);
                break;
            }
        }

        // 保存
        Save();
    }

    //*****************************************************************************
    // ソート処理
    //*****************************************************************************
    void SortRank(void)
    {
        std::sort(m_rankList.begin(), m_rankList.end(),
            [this](const RankData& a, const RankData& b)
            {
                if (m_rankType == RankType::ItemCount)
                {
                    // アイテム数：多い順
                    if (a.items != b.items)
                    {
                        return a.items > b.items;
                    }
                }
                else
                {
                    // クリアタイム：速い順
                    if (a.minutes != b.minutes)
                    {
                        return a.minutes < b.minutes;
                    }

                    if (a.seconds != b.seconds)
                    {
                        return a.seconds < b.seconds;
                    }
                }

                // 同点時の保険
                return a.items > b.items;
            });
    }

};

#endif
