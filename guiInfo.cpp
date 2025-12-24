//=============================================================================
//
// GUI情報表示処理 [guiInfo.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "guiInfo.h"
#include "imguimaneger.h"
#include "player.h"
#include "enemy.h"
#include "manager.h"

//=============================================================================
// プレイヤーのデバッグ情報の表示処理
//=============================================================================
void CGuiInfo::PlayerInfo(void)
{
	CPlayer* pPlayer = CCharacterManager::GetInstance().GetCharacter<CPlayer>();

	if (!pPlayer)
	{
		return;
	}

	D3DXVECTOR3 pos = pPlayer->GetPos();
	D3DXVECTOR3 rot = pPlayer->GetRot();
	D3DXVECTOR3 size = pPlayer->GetSize();

	if (ImGui::TreeNode("Player Info"))
	{
		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		// HP
		ImGui::Text("HP : %.1f / %.1f", pPlayer->GetHp(), pPlayer->GetMaxHp());

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		//*********************************************************************
		// POS の情報
		//*********************************************************************

		// 位置
		ImGui::Text("POS"); ImGui::SameLine(70); // ラベル位置微調整

		ImGui::Text("X:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80); // 幅を狭く
		ImGui::DragFloat("##Player_pos_x", &pos.x, 1.0f, -1000.0f, 1000.0f, "%.1f");

		ImGui::SameLine();
		ImGui::Text("Y:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Player_pos_y", &pos.y, 1.0f, -1000.0f, 1000.0f, "%.1f");

		ImGui::SameLine();
		ImGui::Text("Z:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Player_pos_z", &pos.z, 1.0f, -1000.0f, 1000.0f, "%.1f");

		//*********************************************************************
		// ROT の情報
		//*********************************************************************

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		// 向き
		ImGui::Text("ROT"); ImGui::SameLine(70); // ラベル位置微調整

		ImGui::Text("X:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80); // 幅を狭く
		ImGui::DragFloat("##Player_rot_x", &rot.x, 0.1f, -D3DX_PI, D3DX_PI, "%.1f");

		ImGui::SameLine();
		ImGui::Text("Y:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Player_rot_y", &rot.y, 0.1f, -D3DX_PI, D3DX_PI, "%.1f");

		ImGui::SameLine();
		ImGui::Text("Z:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Player_rot_z", &rot.z, 0.1f, -D3DX_PI, D3DX_PI, "%.1f");

		//*********************************************************************
		// SIZE の情報
		//*********************************************************************

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		// ラベル
		ImGui::Text("SIZE"); ImGui::SameLine(70); // ラベルの位置ちょっと調整

		// X
		ImGui::Text("X:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Player_size_x", &size.x, 0.1f, -100.0f, 100.0f, "%.1f");

		// Y
		ImGui::SameLine();
		ImGui::Text("Y:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Player_size_y", &size.y, 0.1f, -100.0f, 100.0f, "%.1f");

		// Z
		ImGui::SameLine();
		ImGui::Text("Z:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Player_size_z", &size.z, 0.1f, -100.0f, 100.0f, "%.1f");

		ImGui::TreePop(); // 閉じる
	}
}
//=============================================================================
// 敵のデバッグ情報の表示処理
//=============================================================================
void CGuiInfo::EnemyInfo(void)
{
	CEnemy* pEnemy = CCharacterManager::GetInstance().GetCharacter<CEnemy>();

	if (!pEnemy)
	{
		return;
	}

	D3DXVECTOR3 pos = pEnemy->GetPos();
	D3DXVECTOR3 rot = pEnemy->GetRot();
	D3DXVECTOR3 size = pEnemy->GetSize();

	if (ImGui::TreeNode("Enemy Info"))
	{
		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		// HP
		ImGui::Text("HP : %.1f / %.1f", pEnemy->GetHp(), pEnemy->GetMaxHp());

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		//*********************************************************************
		// POS の情報
		//*********************************************************************

		// 位置
		ImGui::Text("POS"); ImGui::SameLine(70); // ラベル位置微調整

		ImGui::Text("X:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80); // 幅を狭く
		ImGui::DragFloat("##Enemy_pos_x", &pos.x, 1.0f, -1000.0f, 1000.0f, "%.1f");

		ImGui::SameLine();
		ImGui::Text("Y:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Enemy_pos_y", &pos.y, 1.0f, -1000.0f, 1000.0f, "%.1f");

		ImGui::SameLine();
		ImGui::Text("Z:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Enemy_pos_z", &pos.z, 1.0f, -1000.0f, 1000.0f, "%.1f");

		//*********************************************************************
		// ROT の情報
		//*********************************************************************

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		// 向き
		ImGui::Text("ROT"); ImGui::SameLine(70); // ラベル位置微調整

		ImGui::Text("X:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80); // 幅を狭く
		ImGui::DragFloat("##Enemy_rot_x", &rot.x, 0.1f, -D3DX_PI, D3DX_PI, "%.1f");

		ImGui::SameLine();
		ImGui::Text("Y:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Enemy_rot_y", &rot.y, 0.1f, -D3DX_PI, D3DX_PI, "%.1f");

		ImGui::SameLine();
		ImGui::Text("Z:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Enemy_rot_z", &rot.z, 0.1f, -D3DX_PI, D3DX_PI, "%.1f");

		//*********************************************************************
		// SIZE の情報
		//*********************************************************************

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		// ラベル
		ImGui::Text("SIZE"); ImGui::SameLine(70); // ラベルの位置ちょっと調整

		// X
		ImGui::Text("X:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Enemy_size_x", &size.x, 0.1f, -100.0f, 100.0f, "%.1f");

		// Y
		ImGui::SameLine();
		ImGui::Text("Y:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Enemy_size_y", &size.y, 0.1f, -100.0f, 100.0f, "%.1f");

		// Z
		ImGui::SameLine();
		ImGui::Text("Z:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(80);
		ImGui::DragFloat("##Enemy_size_z", &size.z, 0.1f, -100.0f, 100.0f, "%.1f");

		ImGui::TreePop(); // 閉じる
	}
}
//=============================================================================
// 学習AI情報の表示処理
//=============================================================================
void CGuiInfo::AIInfo(void)
{
	CEnemy* pEnemy = CCharacterManager::GetInstance().GetCharacter<CEnemy>();// 敵の取得

	if (!pEnemy )
	{
		return;
	}

	if (ImGui::TreeNode("AI Info"))
	{
		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		// HP
		ImGui::Text("HP : %.1f / %.1f", pEnemy->GetHp(), pEnemy->GetMaxHp());

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける


		ImGui::TreePop(); // 閉じる
	}
}
//=============================================================================
// カメラのデバッグ情報の表示処理
//=============================================================================
void CGuiInfo::CameraInfo(void)
{
	CCamera* pCamera = CManager::GetCamera();

	if (pCamera == nullptr)
	{
		return;
	}

	D3DXVECTOR3 posV = pCamera->GetPosV();
	D3DXVECTOR3 posR = pCamera->GetPosR();
	D3DXVECTOR3 rot = pCamera->GetRot();

	if (ImGui::TreeNode("Camera Info"))
	{
		ImGui::Text("Change Camera Mode : [Press C]");

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		ImGui::Text("PosV Move : [Press L ALT & MouseButton L]");
		ImGui::Text("PosR Move : [Press MouseButton R]");
		ImGui::Text("Zoom : [MouseWheel]");

		ImGui::Dummy(ImVec2(0.0f, 10.0f)); // 空白を空ける

		// 視点の位置
		ImGui::Text("PosV [X:%.1f Y:%.1f Z:%.1f]", posV.x, posV.y, posV.z);

		// 注視点の位置
		ImGui::Text("PosR [X:%.1f Y:%.1f Z:%.1f]", posR.x, posR.y, posR.z);

		// 角度
		ImGui::Text("Rot  [X:%.2f Y:%.2f Z:%.2f]", rot.x, rot.y, rot.z);

		ImGui::TreePop(); // 閉じる
	}
}