#include "ShotBase.h"
#include "../Collider/ColliderCapsule.h"
#include "../Collider/ColliderSphere.h"
#include "../Common/Transform.h"

ShotBase::ShotBase()
{
}

ShotBase::~ShotBase()
{
	Clear();
}

void ShotBase::SpawnShot(const VECTOR& pos, const VECTOR& vel, float radius, float life)
{
	SphereShot s;
	s.pos = pos;
	s.r = radius;
	s.vel = vel;
	s.life = life;
	shots_.emplace_back(s);
}

void ShotBase::Update(float dt)
{
	for (auto it = shots_.begin(); it != shots_.end(); )
	{
		// ˆÚ“®
		it->pos = VAdd(it->pos, VScale(it->vel, dt));

		// Žõ–½Œ¸­
		it->life -= dt;

		// Žõ–½Ø‚ê‚Åíœ
		if (it->life <= 0.0f)
		{
			it = shots_.erase(it);
		}
		else { ++it; }
	}
}

bool ShotBase::Collision(const ColliderCapsule* targetCollider)
{
	if (targetCollider == nullptr || !targetCollider->IsValid()) return false;

	bool isHit = false;
	for (auto it = shots_.begin(); it != shots_.end(); )
	{
		// ’e‚ÌÀ•W‚ð’Ç]æ‚Æ‚·‚é“GUŒ‚—p‹…ƒRƒ‰ƒCƒ_
		Transform shotTransform;
		shotTransform.pos = it->pos;
		ColliderSphere shotCollider(
			ColliderBase::TAG::ENEMY_ATTACK,
			&shotTransform,
			SchoolUtility::VECTOR_ZERO,
			it->r);

		if (shotCollider.IsHit(targetCollider))
		{
			// –½’†‚µ‚½’e‚ÍÁ–Å
			it = shots_.erase(it);
			isHit = true;
		}
		else
		{
			++it;
		}
	}
	return isHit;
}

void ShotBase::Draw() const
{
	for (const auto& s : shots_)
	{
		if (s.life > 0.0f)
		DrawSphere3D(s.pos, s.r, 8, 0xff0000, 0xff0000, true);
	}
}

void ShotBase::Clear()
{
	shots_.clear();
}
