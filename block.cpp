//=============================================================================
//
// ブロック処理 [block.cpp]
// Author : RIKU TANEKAWA
//
//=============================================================================

//*****************************************************************************
// インクルードファイル
//*****************************************************************************
#include "block.h"
#include "manager.h"
#include "particle.h"
#include "game.h"
#include "result.h"
#include "blocklist.h"
#include "grass.h"

//*****************************************************************************
// 静的メンバ変数宣言
//*****************************************************************************
std::unordered_map<CBlock::TYPE, BlockCreateFunc> CBlock::m_BlockFactoryMap = {};

//=============================================================================
// コンストラクタ
//=============================================================================
CBlock::CBlock(int nPriority) : CObjectX(nPriority)
{
	// 値のクリア
	m_col			 = INIT_XCOL;				// 色
	m_baseCol		 = INIT_XCOL;				// ベースの色
	m_bSelected		 = false;					// 選択フラグ
	m_pDebug3D		 = nullptr;					// 3Dデバッグ表示へのポインタ
	m_prevSize		 = INIT_VEC3;				// 前回のサイズ
	m_colliderSize	 = INIT_VEC3;				// コライダーサイズ
	m_colliderOffset = INIT_VEC3;				// コライダーのオフセット
	m_isEditMode	 = false;					// 編集中かどうか
	m_isDead		 = false;					// 削除予約フラグ
}
//=============================================================================
// 生成処理
//=============================================================================
CBlock* CBlock::Create(const char* pFilepath, D3DXVECTOR3 pos, D3DXVECTOR3 rot, D3DXVECTOR3 size, TYPE type)
{
	if (m_BlockFactoryMap.empty())
	{
		// ファクトリー
		InitFactory();
	}

	CBlock* pBlock = nullptr;

	auto it = m_BlockFactoryMap.find(type);
	if (it != m_BlockFactoryMap.end())
	{
		pBlock = it->second();
	}
	else
	{
		pBlock = new CBlock(); // デフォルト基底クラス
	}

	if (!pBlock)
	{
		return nullptr;
	}

	pBlock->SetPos(pos);
	pBlock->SetRot(rot);
	pBlock->SetSize(size);
	pBlock->SetType(type);
	pBlock->SetPath(pFilepath);

	// 初期化失敗時
	if (FAILED(pBlock->Init()))
	{
		return nullptr;
	}

	// 当たり判定の生成
	pBlock->CreatePhysicsFromScale(size);

	return pBlock;
}
//=============================================================================
// ファクトリー
//=============================================================================
void CBlock::InitFactory(void)
{
	// リストを空にする
	m_BlockFactoryMap.clear();

	m_BlockFactoryMap[CBlock::TYPE_WALL_01]			= []() -> CBlock* { return new CWallBlock(); };
	m_BlockFactoryMap[CBlock::TYPE_GRASS_FLOOR]		= []() -> CBlock* { return new CGrassFloorBlock(); };
	m_BlockFactoryMap[CBlock::TYPE_TORCH_01]		= []() -> CBlock* { return new CTorchBlock(); };
	m_BlockFactoryMap[CBlock::TYPE_GRASS]			= []() -> CBlock* { return new CGrassBlock(); };
	m_BlockFactoryMap[CBlock::TYPE_TORCH_01]		= []() -> CBlock* { return new CTorchBlock(); };
	m_BlockFactoryMap[CBlock::TYPE_ROCK]			= []() -> CBlock* { return new CRockBlock(); };
	m_BlockFactoryMap[CBlock::TYPE_BLOSSOM_TREE]	= []() -> CBlock* { return new CBlossomTreeBlock(); };
	m_BlockFactoryMap[CBlock::TYPE_BURIED_TREASURE] = []() -> CBlock* { return new CBuriedTreasureBlock(); };
	m_BlockFactoryMap[CBlock::TYPE_DOOR]			= []() -> CBlock* { return new CDoorBlock(); };
	m_BlockFactoryMap[CBlock::TYPE_GHOSTOBJECT]		= []() -> CBlock* { return new CExitBlock(); };
	m_BlockFactoryMap[CBlock::TYPE_GATE]			= []() -> CBlock* { return new CGateBlock(); };
	m_BlockFactoryMap[CBlock::TYPE_GEAR]			= []() -> CBlock* { return new CGearBlock(); };
}
//=============================================================================
// 初期化処理
//=============================================================================
HRESULT CBlock::Init(void)
{
	// オブジェクトXの初期化処理
	CObjectX::Init();

	// マテリアル色をブロックの色に設定
	m_col = GetMaterialColor();
	m_col = m_baseCol;              // 現在の色にも一度入れておく

	return S_OK;
}
//=============================================================================
// 終了処理
//=============================================================================
void CBlock::Uninit(void)
{
	ReleasePhysics();

	// オブジェクトXの終了処理
	CObjectX::Uninit();
}
//=============================================================================
// 更新処理
//=============================================================================
void CBlock::Update(void)
{
	// 静的ブロックは Transform を手動で更新
	if (!IsDynamicBlock() || IsEditMode())
	{
		D3DXVECTOR3 Pos = GetPos() + m_colliderOffset;
		D3DXVECTOR3 Rot = GetRot();

		btTransform trans;
		trans.setIdentity();

		D3DXMATRIX matRot;
		D3DXMatrixRotationYawPitchRoll(&matRot, Rot.y, Rot.x, Rot.z);

		D3DXQUATERNION dq;
		D3DXQuaternionRotationMatrix(&dq, &matRot);

		btQuaternion q(dq.x, dq.y, dq.z, dq.w);
		q.normalize();// 正規化

		trans.setOrigin(btVector3(Pos.x, Pos.y, Pos.z));
		trans.setRotation(q);

		if (m_pRigidBody && m_pRigidBody->getMotionState())
		{
			m_pRigidBody->setWorldTransform(trans);
			m_pRigidBody->getMotionState()->setWorldTransform(trans);
		}
	}
	else
	{
		// 動的ブロックは物理の変換を取得して反映

		if (!m_pRigidBody || !m_pRigidBody->getMotionState())
		{
			return;
		}

		btTransform trans;

		m_pRigidBody->getMotionState()->getWorldTransform(trans);

		btVector3 pos = trans.getOrigin();

		// 位置セット（オフセット差し引き）
		D3DXVECTOR3 newPos(pos.x(), pos.y(), pos.z());
		SetPos(newPos - m_colliderOffset);

		btQuaternion rot = m_pRigidBody->getOrientation();
		rot.normalize();

		// Z軸反転で座標系補正
		D3DXQUATERNION dq(rot.x(), rot.y(), rot.z(), rot.w());

		// 回転行列に変換
		D3DXMATRIX matRot;
		D3DXMatrixRotationQuaternion(&matRot, &dq);

		// 行列 -> オイラー角（XYZ順）
		D3DXVECTOR3 euler;
		float sy = -matRot._32; // 右手座標

		// Clamp範囲を厳密にして異常角回避
		sy = std::max(-1.0f, std::min(1.0f, sy));
		euler.x = asinf(sy);  // pitch (X)

		// cos(pitch) が0に近いとジンバルロックなので、その回避処理
		if (fabsf(cosf(euler.x)) > 1e-4f)
		{
			euler.y = atan2f(matRot._31, matRot._33);  // yaw (Y)
			euler.z = atan2f(matRot._12, matRot._22);  // roll (Z)
		}
		else
		{
			euler.y = 0.0f;
			euler.z = atan2f(-matRot._21, matRot._11); // 代替値（Rollだけ抽出）
		}

		// 連続補正
		static D3DXVECTOR3 prevEuler(0, 0, 0);
		auto FixAngleJump = [](float prev, float current) -> float

		{
			if (_isnan(current))
			{
				return prev;
			}

			float diff = current - prev;

			if (diff > D3DX_PI)
			{
				current -= 2 * D3DX_PI;
			}
			else if (diff < -D3DX_PI)
			{
				current += 2 * D3DX_PI;
			}

			return current;
		};

		euler.x = FixAngleJump(prevEuler.x, euler.x);
		euler.y = FixAngleJump(prevEuler.y, euler.y);
		euler.z = FixAngleJump(prevEuler.z, euler.z);

		prevEuler = euler;

		// セット
		SetRot(euler);
	}
}
//=============================================================================
// コライダーの更新処理
//=============================================================================
void CBlock::UpdateCollider(void)
{
	if (!m_pRigidBody)
	{
		return;
	}

	// 位置の取得
	D3DXVECTOR3 Pos = GetPos();

	// 削除して再生成
	ReleasePhysics();
	CreatePhysics(Pos, m_colliderSize);
}
//=============================================================================
// 描画処理
//=============================================================================
void CBlock::Draw(void)
{
	// オブジェクトXの描画処理
	CObjectX::Draw();
}
//=============================================================================
// 当たり判定描画処理
//=============================================================================
void CBlock::DrawCollider(void)
{
	if (m_pRigidBody)
	{
		// コライダーの描画
		m_pDebug3D->DrawBlockCollider(m_pRigidBody, D3DXCOLOR(0.0f, 1.0f, 0.3f, 1.0f));
	}
}
//=============================================================================
// 色の取得
//=============================================================================
D3DXCOLOR CBlock::GetCol(void) const
{
	if (m_bSelected)
	{// 赤くする
		return D3DXCOLOR(1.0f, 0.0f, 0.0f, 0.6f);
	}
	else
	{// 無補正
		return D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f); 
	}
}
//=============================================================================
// 当たり判定の生成処理
//=============================================================================
void CBlock::CreatePhysics(const D3DXVECTOR3& pos, const D3DXVECTOR3& size)
{
	// Physicsの破棄
	ReleasePhysics();

	m_colliderSize = size;
	m_pShape = CreateCollisionShape(size);// コライダーの生成

	btTransform transform;
	transform.setIdentity();
	btVector3 origin(pos.x + m_colliderOffset.x, pos.y + m_colliderOffset.y, pos.z + m_colliderOffset.z);
	transform.setOrigin(origin);

	// 回転を反映
	D3DXVECTOR3 euler = GetRot();
	D3DXMATRIX matRot;
	D3DXMatrixRotationYawPitchRoll(&matRot, euler.y, euler.x, euler.z);

	D3DXQUATERNION dq;
	D3DXQuaternionRotationMatrix(&dq, &matRot);
	btQuaternion quat(dq.x, dq.y, dq.z, dq.w);
	transform.setRotation(quat);

	// 編集中は強制的に静的
	btScalar mass = m_isEditMode ? GetMass() : (!IsDynamicBlock() ? 0.0f : GetMass());
	btVector3 inertia(0, 0, 0);

	if (mass != 0.0f)
	{
		m_pShape->calculateLocalInertia(mass, inertia);
	}

	btDefaultMotionState* motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, m_pShape, inertia);

	// リジッドボディの生成
	m_pRigidBody = new btRigidBody(rbInfo);
	m_pRigidBody->setUserPointer(this);

	int flags = m_pRigidBody->getCollisionFlags();
	flags |= GetCollisionFlags();

	// 派生クラス側の特殊処理
	OnPhysicsCreated();

	m_pRigidBody->setActivationState(DISABLE_DEACTIVATION);	// スリープ状態にしない
	m_pRigidBody->setCollisionFlags(flags);					// コリジョンフラグ
	m_pRigidBody->setLinearFactor(GetLinearFactor());		// 動く方向
	m_pRigidBody->setAngularFactor(GetAngularFactor());		// 回転する方向
	m_pRigidBody->setFriction(GetFriction());				// 摩擦
	m_pRigidBody->setRollingFriction(GetRollingFriction());	// 転がり摩擦
	m_pRigidBody->setDamping(0.1f, 0.5f);					// 減衰(抵抗)linearDamping, angularDamping

	// 物理ワールドの取得
	btDiscreteDynamicsWorld* pWorld = CManager::GetPhysicsWorld();

	if (pWorld != nullptr)
	{
		pWorld->addRigidBody(m_pRigidBody);
	}
}
//=============================================================================
// スケールによるコライダーの生成処理
//=============================================================================
void CBlock::CreatePhysicsFromScale(const D3DXVECTOR3& scale)
{
	// モデルの元サイズの取得
	D3DXVECTOR3 modelSize = GetModelSize();

	D3DXVECTOR3 newColliderSize =
	{
		modelSize.x * scale.x,
		modelSize.y * scale.y,
		modelSize.z * scale.z
	};

	CreatePhysics(GetPos(), newColliderSize); // 再生成
}
//=============================================================================
// 当たり判定の手動設定用
//=============================================================================
void CBlock::SetColliderManual(const D3DXVECTOR3& newSize)
{
	m_colliderSize = newSize;

	ReleasePhysics();							// 以前の剛体を削除

	CreatePhysics(GetPos(), m_colliderSize);	// 再生成
}
//=============================================================================
// Physicsの破棄
//=============================================================================
void CBlock::ReleasePhysics(void)
{
	auto world = CManager::GetPhysicsWorld();

	// 剛体やシェイプを消す前に派生クラス用の特殊処理破棄を呼ぶ
	OnPhysicsReleased();

	// リジッドボディの破棄
	if (m_pRigidBody)
	{
		if (world)
		{
			world->removeRigidBody(m_pRigidBody);
		}

		delete m_pRigidBody->getMotionState();
		delete m_pRigidBody;
		m_pRigidBody = nullptr;
	}

	// シェイプの破棄
	if (m_pShape)
	{
		delete m_pShape;
		m_pShape = nullptr;
	}
}
//=============================================================================
// エディター中かどうかでキネマティックにするか判定する処理
//=============================================================================
void CBlock::SetEditMode(bool enable)
{
	m_isEditMode = enable;

	if (!m_pRigidBody)
	{
		return;
	}

	if (enable)
	{
		// キネマティックにする
		m_pRigidBody->setCollisionFlags(m_pRigidBody->getCollisionFlags() | btCollisionObject::CF_KINEMATIC_OBJECT);
		m_pRigidBody->setActivationState(DISABLE_DEACTIVATION);
	}
	else
	{
		// キネマティック解除
		m_pRigidBody->setCollisionFlags(m_pRigidBody->getCollisionFlags() & ~btCollisionObject::CF_KINEMATIC_OBJECT);
		m_pRigidBody->setActivationState(ACTIVE_TAG);
	}
}
//=============================================================================
// ワールドマトリックスの取得
//=============================================================================
D3DXMATRIX CBlock::GetWorldMatrix(void)
{
	D3DXMATRIX matScale, matRot, matTrans;

	// スケール行列
	D3DXVECTOR3 scale = GetSize(); // 拡大率
	D3DXMatrixScaling(&matScale, scale.x, scale.y, scale.z);

	// 回転行列
	D3DXVECTOR3 rot = GetRot(); // ラジアン角
	D3DXMatrixRotationYawPitchRoll(&matRot, rot.y, rot.x, rot.z);

	// 平行移動行列
	D3DXVECTOR3 pos = GetPos();
	D3DXMatrixTranslation(&matTrans, pos.x, pos.y, pos.z);

	// 合成：S * R * T
	D3DXMATRIX world = matScale * matRot * matTrans;

	return world;
}
//=============================================================================
// リスポーン処理
//=============================================================================
void CBlock::Respawn(D3DXVECTOR3 resPos)
{
	// 動かすためにキネマティックにする
	SetEditMode(true);

	// リスポーン位置に設定
	SetPos(resPos);
	SetRot(D3DXVECTOR3(0, 0, 0));

	// コライダーの更新
	UpdateCollider();

	// 動的に戻す
	SetEditMode(false);
}
//=============================================================================
// 簡易の当たり判定処理(イベント用)
//=============================================================================
bool CBlock::IsHitOBBvsAABB(const OBB& obb, const D3DXVECTOR3& aabbMin, const D3DXVECTOR3& aabbMax)
{
	// まずAABBの中心と半径を求める
	D3DXVECTOR3 aabbCenter = (aabbMin + aabbMax) * 0.5f;
	D3DXVECTOR3 aabbHalf = (aabbMax - aabbMin) * 0.5f;

	// OBB→AABB座標に変換する行列を求める
	D3DXVECTOR3 diff = aabbCenter - obb.center;

	// 分離軸定理（SAT）
	for (int i = 0; i < 3; i++)
	{
		// OBBの軸に対する投影距離
		float proj = fabs(D3DXVec3Dot(&diff, &obb.axis[i]));
		float rA = obb.halfSize[i];
		float rB;
		{
			D3DXVECTOR3 axisX(1, 0, 0);
			D3DXVECTOR3 axisY(0, 1, 0);
			D3DXVECTOR3 axisZ(0, 0, 1);

			rB =
				aabbHalf.x * fabs(D3DXVec3Dot(&axisX, &obb.axis[i])) +
				aabbHalf.y * fabs(D3DXVec3Dot(&axisY, &obb.axis[i])) +
				aabbHalf.z * fabs(D3DXVec3Dot(&axisZ, &obb.axis[i]));
		}

		if (proj > rA + rB)
		{
			return false; // 軸で分離できた → 非衝突
		}
	}

	// AABBの3軸でもチェック
	const D3DXVECTOR3 aabbAxes[3] =
	{
		{1, 0, 0},
		{0, 1, 0},
		{0, 0, 1}
	};

	for (int i = 0; i < 3; i++)
	{
		float proj = fabs(D3DXVec3Dot(&diff, &aabbAxes[i]));
		float rA =
			obb.halfSize.x * fabs(D3DXVec3Dot(&obb.axis[0], &aabbAxes[i])) +
			obb.halfSize.y * fabs(D3DXVec3Dot(&obb.axis[1], &aabbAxes[i])) +
			obb.halfSize.z * fabs(D3DXVec3Dot(&obb.axis[2], &aabbAxes[i]));
		float rB = aabbHalf[i];

		if (proj > rA + rB)
		{
			return false;
		}
	}

	return true;
}