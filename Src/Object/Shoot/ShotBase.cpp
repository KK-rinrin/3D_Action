#include "ShotBase.h"

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