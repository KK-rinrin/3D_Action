#include <cmath>
#include "../Common/Transform.h"
#include "ColliderCapsule.h"
#include "ColliderCircle.h"
#include "../../Utility/SchoolUtility.h"

ColliderCircle::ColliderCircle(
	TAG tag, const Transform* follow, const VECTOR& localPos, float radius)
	: ColliderBase(SHAPE::CIRCLE, tag, follow)
	, localPos_(localPos)
	, radius_(radius)
{
}

ColliderCircle::~ColliderCircle(void)
{
}

VECTOR ColliderCircle::GetPos(void) const
{
	return GetRotPos(localPos_);
}

float ColliderCircle::GetRadius(void) const
{
	return radius_;
}

bool ColliderCircle::IsHit(const ColliderCapsule* collider) const
{
	if (collider == nullptr) return false;

	VECTOR center = GetPos();
	VECTOR lineStart = collider->GetPosDown();
	VECTOR lineEnd = collider->GetPosTop();
	center.y = 0.0f;
	lineStart.y = 0.0f;
	lineEnd.y = 0.0f;

	VECTOR line = VSub(lineEnd, lineStart);
	const float lineLengthSq = VDot(line, line);
	float rate = 0.0f;
	if (lineLengthSq > 0.0f)
	{
		rate = VDot(VSub(center, lineStart), line) / lineLengthSq;
		rate = SchoolUtility::Clamp(rate, 0.0f, 1.0f);
	}

	const VECTOR nearest = VAdd(lineStart, VScale(line, rate));
	const VECTOR diff = VSub(center, nearest);
	const float hitRadius = radius_ + collider->GetRadius();
	return VDot(diff, diff) <= hitRadius * hitRadius;
}

VECTOR ColliderCircle::GetPosPushBackAlongNormal(
	const MV1_COLL_RESULT_POLY& hitColPoly,
	int maxTryCnt,
	float pushDistance) const
{
	return follow_->pos;
}

void ColliderCircle::DrawDebug(int color)
{
	static constexpr int DIV_NUM = 24;
	const VECTOR center = GetPos();
	VECTOR previous = VGet(center.x + radius_, center.y, center.z);

	for (int i = 1; i <= DIV_NUM; ++i)
	{
		const float angle =
			DX_TWO_PI_F * static_cast<float>(i) / static_cast<float>(DIV_NUM);
		const VECTOR current = VGet(
			center.x + cosf(angle) * radius_,
			center.y,
			center.z + sinf(angle) * radius_);
		DrawLine3D(previous, current, color);
		previous = current;
	}
}
