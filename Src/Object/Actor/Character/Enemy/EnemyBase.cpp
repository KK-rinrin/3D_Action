#include "EnemyBase.h"
#include "../../../../Utility/SchoolUtility.h"
#include "../../../Collider/ColliderSphere.h"
#include "../../../Collider/ColliderCapsule.h"

EnemyBase::EnemyBase(const EnemyBase::EnemyData& data)
	:
	type_(data.type),
	hp_(data.hp),
	stateBase_(-1),
	defaultPos_(data.defaultPos),
	moveRadius_(data.radius),
	isHit_(false),
	targetTransform_(nullptr)
{
	// 初期座標の設定
	transform_.pos = defaultPos_;
}

EnemyBase::~EnemyBase(void)
{
}

void EnemyBase::SetTargetTransform(const Transform* targetTransform)
{
	targetTransform_ = targetTransform;
}

void EnemyBase::Draw(void)
{
	CharacterBase::Draw();

#ifdef _DEBUG

	// 移動可能範囲のデバッグ描画
	DrawSphere3D(defaultPos_, moveRadius_, 16, 0x000099, 0x000099, false);

#endif // _DEBUG
}

void EnemyBase::CollisionPost(void)
{
	// 衝突判定(プレイヤーの武器)
	CollisionWeapon();
}

bool EnemyBase::InMovableRange(void) const
{
	bool ret = false;

	// 初期位置からの距離
	float dis = static_cast<float>(
		SchoolUtility::SqrMagnitude(defaultPos_, transform_.pos));

	// 指定距離判定
	if (dis < moveRadius_ * moveRadius_)
	{
		return true;
	}

	return ret;
}

void EnemyBase::ChangeState(int state)
{
	stateBase_ = state;

	// 各状態遷移の初期処理
	stateChanges_[stateBase_]();
}

bool EnemyBase::InSearch(float viewRange, float viewAngle)
{
	return false;
}

void EnemyBase::CollisionWeapon(void)
{
	if (isHit_) return;

	for (const auto& hitCol : hitColliders_)
	{
		// 有効でなければ処理を飛ばす
		if (!hitCol->IsValid()) continue;

		// プレイヤー武器以外は処理を飛ばす
		if (hitCol->GetTag() != ColliderBase::TAG::PLAYER_WEAPON) continue;

		// 派生クラスへキャスト
		const ColliderCapsule* colliderCapsule =
			dynamic_cast<const ColliderCapsule*>(hitCol);

		if (colliderCapsule == nullptr) continue;

		ColliderCapsule* colCap = dynamic_cast<ColliderCapsule*>
			(ownColliders_.at(static_cast<int>(COLLIDER_TYPE::CAPSULE)));

		if (colliderCapsule->IsHit(colCap))
		{
			// この中に当たった時の処理を追加
			isHit_ = true;

			return;
		}
	}
}
