//=============================================================================
//
// 敵の学習AI処理 [enemyAI.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "enemyAI.h"
#include "player.h"
#include "enemy.h"
#include "meshcylinder.h"
#include "time.h"
#include "blockmanager.h"
#include "SEpopupeffect.h"
#include "sound.h"
#include "manager.h"

//=============================================================================
// リーダー敵AIコンストラクタ
//=============================================================================
CEnemyAI_Leader::CEnemyAI_Leader()
{
	// 値のクリア
    m_logTimer = 0;                 // 記録タイマー
    m_prevInSight = false;			// 直前に視界に入ったか
    m_soundTimer = 0;
    m_soundCount = 0;
}
//=============================================================================
// リーダー敵AIデストラクタ
//=============================================================================
CEnemyAI_Leader::~CEnemyAI_Leader()
{
	// なし
}
//=============================================================================
// リーダー敵AI更新処理
//=============================================================================
void CEnemyAI_Leader::Update(CEnemy* pEnemy, CPlayer* pPlayer)
{
    if (!pEnemy || !pPlayer)
    {
        return;
    }

    // 行動を記録
    RecordPlayerAction(pEnemy, pPlayer);

    // 特定のブロックに当たったか判定するため、ブロックマネージャーを取得する
    CBlockManager* pBlockManager = CGame::GetBlockManager();

    // 時間の割合を取得
    float progress = CGame::GetTime()->GetProgress(); // 0.0～0.1
    bool isNight = (progress < 0.30f || progress >= 0.90f);

    // プレイヤーとの距離を算出
    D3DXVECTOR3 diff = pPlayer->GetPos() - pEnemy->GetPos();
    float distance = D3DXVec3Length(&diff);

    // 特定のブロックに当たっているか判定する
    bool playerInGrass = pBlockManager->IsPlayerInGrass();
    bool playerInTorch = pBlockManager->IsPlayerInTorch() && isNight;
    bool playerInWater = pBlockManager->IsPlayerInWater();

    // 視界距離と角度
    float range = 265.0f;
    float angle = 115.0f;

    if (playerInTorch)// 灯籠の近くにいるときは視界を広げる
    {
        range = 220.0f;
        angle = 100.0f;
    }
    else if (playerInGrass)// プレイヤーが草にいるときは視界を狭める
    {
        range = 100.0f;
        angle = 100.0f;
    }

    // 視界距離と角度の設定
    pEnemy->SetSightRange(range);
    pEnemy->SetSightAngle(D3DXToRadian(angle));

    // 音を立てた または 視界に入った回数に応じて命令(サブ敵をそこに向かわせる)確率を上げる
    float prob = CalcSoundProbability(m_log.makeSoundCount);

    // プレイヤーの条件
    bool playerCondition = !pPlayer->IsStealth() && pPlayer->GetIsMoving() &&
        !pPlayer->GetMotion()->IsCurrentMotion(CPlayer::DAMAGE);

    // 当たった対象に応じてテクスチャパスを変える
    const char* path = "data/TEXTURE/popup_01.png";

    if (playerInGrass)// 草
    {
        path = "data/TEXTURE/popup_01.png";
    }
    else if (playerInWater)// 水
    {
        path = "data/TEXTURE/popup_02.png";
    }

    // 特定のオブジェクトに接触かつ忍び足じゃなかったら
    if (playerCondition && (playerInGrass || playerInWater))
    {
        m_soundTimer++;

        // プレイヤーの位置を取得
        D3DXVECTOR3 pos = pPlayer->GetPos();
        pos.y += 40.0f;// 少し上げる

        if (m_soundTimer <= 15)
        {
            return;
        }

        m_soundTimer = 0;
        m_soundCount++;

        // 音発生数の設定
        pEnemy->SetSoundCount(m_soundCount);

        // 音の取得
        CSound* pSound = CManager::GetSound();

        if (pSound && playerInGrass)        // 草SEの再生
        {
            pSound->Play(CSound::SOUND_LABEL_GRASS);
        }
        else if (pSound && playerInWater)   // 水SEの再生
        {
            pSound->Play(CSound::SOUND_LABEL_WATER);
        }

        // 波紋の生成
        CMeshCylinder::Create(pos, D3DXCOLOR(1.0f, 0.0f, 0.0f, 1.0f), 12.0f, 8.0f, 0.8f, 120, 0.008f);

        // 効果音ポップアップエフェクトの生成
        CSEPopupEffect::Create(path, pos, D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f), 40);

        // 音の位置を設定
        pEnemy->OnSoundHeard(pPlayer->GetPos());

        if (PROBABILITY_THRESHOLD <= prob)
        {
            // 命令状態
            pEnemy->SetRequestedAction(CEnemy::AI_ORDER);
        }
        else
        {
            // 疑い状態
            pEnemy->SetRequestedAction(CEnemy::AI_DOUBT);
        }
    }
    else
    {
        // 移動状態
        pEnemy->SetRequestedAction(CEnemy::AI_MOVE);
    }

    // 視界に入ったら
    if (pEnemy->IsPlayerInSight(pPlayer) || distance < TRIGGER_DISTANCE)
    {
        // 発見状態
        pEnemy->SetRequestedAction(CEnemy::AI_DISCOVER);
    }
}
//=============================================================================
// リーダー敵AIのプレイヤーの行動記録処理
//=============================================================================
void CEnemyAI_Leader::RecordPlayerAction(CEnemy* pEnemy, CPlayer* pPlayer)
{
    // 特定のブロックに当たったか判定するため、ブロックマネージャーを取得する
    CBlockManager* pBlockManager = CGame::GetBlockManager();

    // 特定のブロックに当たっているか判定する
    bool playerInGrass = pBlockManager->IsPlayerInGrass();
    bool playerInWater = pBlockManager->IsPlayerInWater();

    // 音カウントの加算
    {
        // 音を立てていたら一定間隔で音カウントを増やす
        if (!pPlayer->IsStealth() && pPlayer->GetIsMoving() &&
            (playerInGrass || playerInWater))
        {
            m_logTimer++;// フレーム加算

            if (m_logTimer <= LOG_TIME)
            {
                return;
            }

            // 音カウントを加算
            m_log.makeSoundCount++;

            m_logTimer = 0;
        }
        else
        {
            m_logTimer = 0;
        }
    }

    // 視界カウントの加算
    {
        // 視界に入った瞬間(1回)だけ通す
        bool n = pEnemy->IsPlayerInSight(pPlayer);

        if (n && !m_prevInSight)
        {
            // 視界カウントを加算
            m_log.insightCount++;
        }

        m_prevInSight = n;
    }
}


//=============================================================================
// サブ敵AIのコンストラクタ
//=============================================================================
CEnemyAI_Sub::CEnemyAI_Sub()
{
    // 値のクリア
    m_logTimer = 0;
    m_prevInSight = false;			// 直前に視界に入ったか
}
//=============================================================================
// サブ敵AIのデストラクタ
//=============================================================================
CEnemyAI_Sub::~CEnemyAI_Sub()
{
    // なし
}
//=============================================================================
// サブ敵AIの更新処理
//=============================================================================
void CEnemyAI_Sub::Update(CEnemy* pEnemy, CPlayer* pPlayer)
{
    if (!pEnemy || !pPlayer)
    {
        return;
    }

    // 行動を記録
    RecordPlayerAction(pEnemy, pPlayer);

    // プレイヤーとの距離を算出
    D3DXVECTOR3 diff = pPlayer->GetPos() - pEnemy->GetPos();
    float distance = D3DXVec3Length(&diff);

    // 音を立てた回数に応じて命令(サブ敵をそこに向かわせる)確率を上げていく
    float prob = CalcSoundProbability(m_log.makeSoundCount);

    // 独自行動へ移行する確率判定
    if (PROBABILITY_THRESHOLD <= prob)
    {
        // 移動状態
        pEnemy->SetRequestedAction(CEnemy::AI_MOVE);
        return;
    }

    //  一定距離近づいたら
    if (distance < CEnemySub::CHASE_DISTANCE)
    {
        // 追跡状態
        pEnemy->SetRequestedAction(CEnemy::AI_CHASE);
        return;
    }

    // リーダーが命令を出したら
    if (pEnemy->IsLeaderAction(CEnemy::AI_ORDER))
    {
        // 音の位置を設定
        pEnemy->OnSoundHeard(pEnemy->GetLastHeardSoundPos());

        // 音源の調査状態
        pEnemy->SetRequestedAction(CEnemy::AI_SOUND_INVESTIGATE);
        return;
    }
    else
    {
        // リーダー追従状態
        pEnemy->SetRequestedAction(CEnemy::AI_FOLLOW);
    }
}
//=============================================================================
// サブ敵AIのプレイヤーの行動記録処理
//=============================================================================
void CEnemyAI_Sub::RecordPlayerAction(CEnemy* pEnemy, CPlayer* pPlayer)
{
    // 特定のブロックに当たったか判定するため、ブロックマネージャーを取得する
    CBlockManager* pBlockManager = CGame::GetBlockManager();

    bool playerInGrass = pBlockManager->IsPlayerInGrass();
    bool playerInWater = pBlockManager->IsPlayerInWater();

    // 音カウントの加算
    {
        // 音を立てていたら一定間隔で音カウントを増やす
        if (!pPlayer->IsStealth() && pPlayer->GetIsMoving() &&
            (playerInGrass || playerInWater))
        {
            m_logTimer++;// フレーム加算

            if (m_logTimer <= LOG_TIME)
            {
                return;
            }

            // 音カウントを加算
            m_log.makeSoundCount++;

            m_logTimer = 0;
        }
        else
        {
            m_logTimer = 0;
        }
    }

    // 視界カウントの加算
    {
        // 視界に入った瞬間(1回)だけ通す
        bool n = pEnemy->IsPlayerInSight(pPlayer);

        if (n && !m_prevInSight)
        {
            // 視界カウントを加算
            m_log.insightCount++;
        }

        m_prevInSight = n;
    }
}
