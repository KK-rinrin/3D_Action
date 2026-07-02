#include "../../../Utility/SchoolUtility.h"
#include "../../../Manager/ResourceManager.h"
#include "WeaponBlade.h"
#include "../../Collider/ColliderCapsule.h"

WeaponBlade::WeaponBlade(const Transform& followTransform, int followFrameId)
	:
	WeaponBase(followTransform, followFrameId)
{
}

WeaponBlade::~WeaponBlade(void)
{
}

void WeaponBlade::Update(void)
{
	WeaponBase::Update();
}

void WeaponBlade::InitLoad(void)
{
	// ƒ‚ƒfƒ‹‚Ìƒ[ƒh
	transform_.SetModel(
		resMng_.Load(ResourceManager::SRC::WEAPON_BLADE).handleId_);
}

void WeaponBlade::InitTransform(void)
{
	transform_.scl = VScale(SchoolUtility::VECTOR_ONE, SCALE);
	transform_.quaRot = Quaternion();
	transform_.quaRotLocal = Quaternion();
	transform_.pos = SchoolUtility::VECTOR_ZERO;

	localPos_ = { -2.0f, 0.0f, -3.0f };
	localRot_ = {
	SchoolUtility::Deg2RadF(0.0f),
	SchoolUtility::Deg2RadF(0.0f),
	SchoolUtility::Deg2RadF(-90.0f)
	};
}

void WeaponBlade::InitCollider(void)
{
	ColliderCapsule* colCapsule = new ColliderCapsule(
		ColliderBase::TAG::PLAYER_WEAPON, &transform_,
		COL_CAPSULE_TOP_LOCAL_POS, COL_CAPSULE_DOWN_LOCAL_POS,
		COL_CAPSULE_RADIUS);

	// Å‰‚Í–³Œø‚É‚µ‚Ä‚¨‚­
	colCapsule->SetValid(false);

	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);
}

void WeaponBlade::InitAnimation(void)
{
}

void WeaponBlade::InitPost(void)
{
}