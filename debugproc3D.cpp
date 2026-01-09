//=============================================================================
//
// 3Dデバッグ表示処理 [debugproc3D.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================
#include "debugproc3D.h"
#include "renderer.h"
#include "manager.h"

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
LPD3DXLINE CDebugProc3D::m_pLine = nullptr;

//=============================================================================
// コンストラクタ
//=============================================================================
CDebugProc3D::CDebugProc3D()
{
	// 値のクリア

}
//=============================================================================
// デストラクタ
//=============================================================================
CDebugProc3D::~CDebugProc3D()
{
	// なし
}
//=============================================================================
// 初期化処理
//=============================================================================
void CDebugProc3D::Init(void)
{
	// デバイスの取得
	LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	if (pDevice != nullptr)
	{
		D3DXCreateLine(pDevice, &m_pLine);
	}
}
//=============================================================================
// 終了処理
//=============================================================================
void CDebugProc3D::Uninit(void)
{
	// ライン表示の破棄
	if (m_pLine != nullptr)
	{
		m_pLine->Release();
		m_pLine = nullptr;
	}
}
//=============================================================================
// ライン描画処理
//=============================================================================
void CDebugProc3D::DrawLine3D(const D3DXVECTOR3& start, const D3DXVECTOR3& end, D3DXCOLOR color)
{
    // デバイスの取得
    LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

	struct VERTEX
	{
		D3DXVECTOR3 pos;
		D3DCOLOR color;
	};

	VERTEX v[2] =
    {
		{ start, color },
		{ end,   color }
	};

	// 頂点フォーマット設定
	pDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

    pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);         // ライトを無効にする
    pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE); // αテストを無効にする

	// ライン描画
	pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, v, sizeof(VERTEX));

    pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);  // αテストを有効にする
    pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);          // ライトを有効にする
}
//=============================================================================
// カプセルコライダー描画処理
//=============================================================================
void CDebugProc3D::DrawCapsuleCollider(btCapsuleShape* capsule, const btTransform& transform, D3DXCOLOR color)
{
    if (!capsule || !m_pLine)
    {
        return;
    }

    // デバイスの取得
    LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

    const int kNumSegments = 12;  // 分割数
    const int kNumRings = 4;      // 縦分割

    float radius = capsule->getRadius();
    float halfHeight = capsule->getHalfHeight();

    // Capsule の中心位置
    btVector3 base = transform.getOrigin();

    // btTransformの回転をD3DXMATRIXへ変換
    btQuaternion q = transform.getRotation();

    D3DXMATRIX matRot;
    D3DXQUATERNION dq(q.x(), q.y(), q.z(), q.w());
    D3DXMatrixRotationQuaternion(&matRot, &dq);

    // 平行移動行列
    D3DXMATRIX matTrans;
    D3DXMatrixTranslation(&matTrans, base.x(), base.y(), base.z());

    // ワールド行列 = 回転 × 平行移動
    D3DXMATRIX matWorld = matRot * matTrans;

    // デバイスにワールド行列を設定
    pDevice->SetTransform(D3DTS_WORLD, &matWorld);

    // カプセル方向はY軸固定
    btVector3 up(0, 1, 0);
    btVector3 side1(1, 0, 0);
    btVector3 side2(0, 0, 1);

    btVector3 top = up * halfHeight;
    btVector3 bottom = -up * halfHeight;

    for (int i = 0; i < kNumSegments; i++)
    {
        float theta1 = (2.0f * D3DX_PI * i) / kNumSegments;
        float theta2 = (2.0f * D3DX_PI * (i + 1)) / kNumSegments;

        btVector3 dir1 = cosf(theta1) * side1 + sinf(theta1) * side2;
        btVector3 dir2 = cosf(theta2) * side1 + sinf(theta2) * side2;

        dir1 *= radius;
        dir2 *= radius;

        // 側面の線
        DrawLine3D(D3DXVECTOR3((top + dir1).x(), (top + dir1).y(), (top + dir1).z()),
            D3DXVECTOR3((bottom + dir1).x(), (bottom + dir1).y(), (bottom + dir1).z()), color);
    }

    // --- 上半球を描く ---
    for (int ring = 0; ring < kNumRings; ring++)
    {
        float theta1 = (D3DX_PI / 2) * (ring / (float)kNumRings);       // 0 ～ π/2
        float theta2 = (D3DX_PI / 2) * ((ring + 1) / (float)kNumRings);

        for (int seg = 0; seg < kNumSegments; seg++)
        {
            float phi1 = (2 * D3DX_PI * seg) / kNumSegments;
            float phi2 = (2 * D3DX_PI * (seg + 1)) / kNumSegments;

            // 球面座標 → 3D位置 (半径1の単位球ベース)
            btVector3 p1 = btVector3(
                cosf(phi1) * sinf(theta1),
                cosf(theta1),
                sinf(phi1) * sinf(theta1)
            );
            btVector3 p2 = btVector3(
                cosf(phi2) * sinf(theta1),
                cosf(theta1),
                sinf(phi2) * sinf(theta1)
            );
            btVector3 p3 = btVector3(
                cosf(phi2) * sinf(theta2),
                cosf(theta2),
                sinf(phi2) * sinf(theta2)
            );
            btVector3 p4 = btVector3(
                cosf(phi1) * sinf(theta2),
                cosf(theta2),
                sinf(phi1) * sinf(theta2)
            );

            // 半球なので、Y成分がup方向 → topにオフセット、半径スケール
            p1 = top + p1 * radius;
            p2 = top + p2 * radius;
            p3 = top + p3 * radius;
            p4 = top + p4 * radius;

            // 4点で四角形 → 2本の線でワイヤーフレーム
            DrawLine3D(D3DXVECTOR3(p1.x(), p1.y(), p1.z()), D3DXVECTOR3(p2.x(), p2.y(), p2.z()), color);
            DrawLine3D(D3DXVECTOR3(p2.x(), p2.y(), p2.z()), D3DXVECTOR3(p3.x(), p3.y(), p3.z()), color);
            DrawLine3D(D3DXVECTOR3(p3.x(), p3.y(), p3.z()), D3DXVECTOR3(p4.x(), p4.y(), p4.z()), color);
            DrawLine3D(D3DXVECTOR3(p4.x(), p4.y(), p4.z()), D3DXVECTOR3(p1.x(), p1.y(), p1.z()), color);
        }
    }

    // --- 下半球を描く ---
    for (int ring = 0; ring < kNumRings; ring++)
    {
        float theta1 = (D3DX_PI / 2) * (ring / (float)kNumRings);
        float theta2 = (D3DX_PI / 2) * ((ring + 1) / (float)kNumRings);

        for (int seg = 0; seg < kNumSegments; seg++)
        {
            float phi1 = (2 * D3DX_PI * seg) / kNumSegments;
            float phi2 = (2 * D3DX_PI * (seg + 1)) / kNumSegments;

            btVector3 p1 = btVector3(
                cosf(phi1) * sinf(theta1),
                -cosf(theta1),  // Y反転
                sinf(phi1) * sinf(theta1)
            );
            btVector3 p2 = btVector3(
                cosf(phi2) * sinf(theta1),
                -cosf(theta1),
                sinf(phi2) * sinf(theta1)
            );
            btVector3 p3 = btVector3(
                cosf(phi2) * sinf(theta2),
                -cosf(theta2),
                sinf(phi2) * sinf(theta2)
            );
            btVector3 p4 = btVector3(
                cosf(phi1) * sinf(theta2),
                -cosf(theta2),
                sinf(phi1) * sinf(theta2)
            );

            p1 = bottom + p1 * radius;
            p2 = bottom + p2 * radius;
            p3 = bottom + p3 * radius;
            p4 = bottom + p4 * radius;

            DrawLine3D(D3DXVECTOR3(p1.x(), p1.y(), p1.z()), D3DXVECTOR3(p2.x(), p2.y(), p2.z()), color);
            DrawLine3D(D3DXVECTOR3(p2.x(), p2.y(), p2.z()), D3DXVECTOR3(p3.x(), p3.y(), p3.z()), color);
            DrawLine3D(D3DXVECTOR3(p3.x(), p3.y(), p3.z()), D3DXVECTOR3(p4.x(), p4.y(), p4.z()), color);
            DrawLine3D(D3DXVECTOR3(p4.x(), p4.y(), p4.z()), D3DXVECTOR3(p1.x(), p1.y(), p1.z()), color);
        }
    }
}
//=============================================================================
// ブロックのコライダー描画処理
//=============================================================================
void CDebugProc3D::DrawBlockCollider(btRigidBody* rigidBody, D3DXCOLOR color)
{
    if (!rigidBody || !m_pLine)
    {
        return;
    }

    // ワールド変換取得
    btTransform transform;

    rigidBody->getMotionState()->getWorldTransform(transform);

    btVector3 pos = transform.getOrigin();
    btQuaternion rot = transform.getRotation();

    // D3DX用の行列作成（回転）
    D3DXQUATERNION dq(rot.x(), rot.y(), rot.z(), rot.w());

    D3DXMATRIX matRot, matTrans, matWorld;

    D3DXMatrixRotationQuaternion(&matRot, &dq);
    D3DXMatrixTranslation(&matTrans, pos.x(), pos.y(), pos.z());

    // ワールド行列（回転 → 平行移動）
    matWorld = matRot * matTrans;

    // デバイス取得
    LPDIRECT3DDEVICE9 pDevice = CManager::GetRenderer()->GetDevice();

    // 行列セット
    pDevice->SetTransform(D3DTS_WORLD, &matWorld);

    btCollisionShape* shape = rigidBody->getCollisionShape();

    switch (shape->getShapeType())
    {
    case BOX_SHAPE_PROXYTYPE:
    {
        const btBoxShape* box = static_cast<const btBoxShape*>(shape);
        btVector3 half = box->getHalfExtentsWithMargin();

        D3DXVECTOR3 v[8] =
        {
            { -half.x(), -half.y(), -half.z() },
            { +half.x(), -half.y(), -half.z() },
            { +half.x(), +half.y(), -half.z() },
            { -half.x(), +half.y(), -half.z() },
            { -half.x(), -half.y(), +half.z() },
            { +half.x(), -half.y(), +half.z() },
            { +half.x(), +half.y(), +half.z() },
            { -half.x(), +half.y(), +half.z() },
        };

        int indices[][2] =
        {
            {0,1},{1,2},{2,3},{3,0},
            {4,5},{5,6},{6,7},{7,4},
            {0,4},{1,5},{2,6},{3,7}
        };

        for (int i = 0; i < 12; i++)
        {
            DrawLine3D(v[indices[i][0]], v[indices[i][1]], color);
        }
        break;
    }
    case SPHERE_SHAPE_PROXYTYPE:
    {
        const btSphereShape* sphere = static_cast<const btSphereShape*>(shape);
        float radius = sphere->getRadius();
        const int kSegments = 16;
        D3DXVECTOR3 center(0, 0, 0);

        for (int i = 0; i < kSegments; i++)
        {
            float t1 = (2.0f * D3DX_PI * i) / kSegments;
            float t2 = (2.0f * D3DX_PI * (i + 1)) / kSegments;

            // XY
            D3DXVECTOR3 p1 = center + D3DXVECTOR3(cosf(t1), sinf(t1), 0) * radius;
            D3DXVECTOR3 p2 = center + D3DXVECTOR3(cosf(t2), sinf(t2), 0) * radius;
            DrawLine3D(p1, p2, color);

            // YZ
            p1 = center + D3DXVECTOR3(0, cosf(t1), sinf(t1)) * radius;
            p2 = center + D3DXVECTOR3(0, cosf(t2), sinf(t2)) * radius;
            DrawLine3D(p1, p2, color);

            // XZ
            p1 = center + D3DXVECTOR3(cosf(t1), 0, sinf(t1)) * radius;
            p2 = center + D3DXVECTOR3(cosf(t2), 0, sinf(t2)) * radius;
            DrawLine3D(p1, p2, color);
        }
        break;
    }
    case COMPOUND_SHAPE_PROXYTYPE:
    {
        const btCompoundShape* compound = static_cast<const btCompoundShape*>(shape);
        int numChildren = compound->getNumChildShapes();

        for (int i = 0; i < numChildren; i++)
        {
            const btCollisionShape* childShape = compound->getChildShape(i);
            const btTransform& childTrans = compound->getChildTransform(i);

            // 子のローカル → ワールド変換
            btTransform worldTrans = transform * childTrans;

            // 子の位置と回転を取得
            btVector3 childPos = worldTrans.getOrigin();
            btQuaternion childRot = worldTrans.getRotation();

            // 行列作成
            //D3DXQUATERNION dq(childRot.x(), childRot.y(), childRot.z(), childRot.w());
            //D3DXMATRIX matRot, matTrans, matWorld;
            D3DXMatrixRotationQuaternion(&matRot, &dq);
            D3DXMatrixTranslation(&matTrans, childPos.x(), childPos.y(), childPos.z());
            matWorld = matRot * matTrans;

            // ワールド行列セット
            pDevice->SetTransform(D3DTS_WORLD, &matWorld);

            // 子コライダーを描画（再帰的に処理）
            int childType = childShape->getShapeType();

            switch (childType)
            {
            case BOX_SHAPE_PROXYTYPE:
            {
                const btBoxShape* box = static_cast<const btBoxShape*>(childShape);
                btVector3 half = box->getHalfExtentsWithMargin();

                D3DXVECTOR3 v[8] =
                {
                    { -half.x(), -half.y(), -half.z() },
                    { +half.x(), -half.y(), -half.z() },
                    { +half.x(), +half.y(), -half.z() },
                    { -half.x(), +half.y(), -half.z() },
                    { -half.x(), -half.y(), +half.z() },
                    { +half.x(), -half.y(), +half.z() },
                    { +half.x(), +half.y(), +half.z() },
                    { -half.x(), +half.y(), +half.z() },
                };

                int indices[][2] =
                {
                    {0,1},{1,2},{2,3},{3,0},
                    {4,5},{5,6},{6,7},{7,4},
                    {0,4},{1,5},{2,6},{3,7}
                };

                for (int j = 0; j < 12; j++)
                {
                    DrawLine3D(v[indices[j][0]], v[indices[j][1]], color);
                }
                break;
            }
            case SPHERE_SHAPE_PROXYTYPE:
            {
                const btSphereShape* sphere = static_cast<const btSphereShape*>(childShape);
                float radius = sphere->getRadius();
                const int kSegments = 16;
                D3DXVECTOR3 center(0, 0, 0);

                for (int j = 0; j < kSegments; j++)
                {
                    float t1 = (2.0f * D3DX_PI * j) / kSegments;
                    float t2 = (2.0f * D3DX_PI * (j + 1)) / kSegments;

                    // XY
                    D3DXVECTOR3 p1 = center + D3DXVECTOR3(cosf(t1), sinf(t1), 0) * radius;
                    D3DXVECTOR3 p2 = center + D3DXVECTOR3(cosf(t2), sinf(t2), 0) * radius;
                    DrawLine3D(p1, p2, color);

                    // YZ
                    p1 = center + D3DXVECTOR3(0, cosf(t1), sinf(t1)) * radius;
                    p2 = center + D3DXVECTOR3(0, cosf(t2), sinf(t2)) * radius;
                    DrawLine3D(p1, p2, color);

                    // XZ
                    p1 = center + D3DXVECTOR3(cosf(t1), 0, sinf(t1)) * radius;
                    p2 = center + D3DXVECTOR3(cosf(t2), 0, sinf(t2)) * radius;
                    DrawLine3D(p1, p2, color);
                }
                break;
            }

            default:
                // 未対応の子タイプ（無視）
                break;
            }
        }
    }
    default:
        break;
    }
}