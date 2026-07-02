#include <cmath>
#include "../Common/Transform.h"
#include "ColliderCapsule.h"
#include "ColliderCone.h"

ColliderCone::ColliderCone(
	TAG tag, const Transform* follow,
	const VECTOR& localPosApex, const VECTOR& localAxis,
	float height, float halfAngleRight, float halfAngleUp)
	:
	ColliderBase(SHAPE::CONE, tag, follow),
	localPosApex_(localPosApex),
	localAxis_(VNorm(localAxis)),
	height_(height),
	halfAngleRight_(halfAngleRight),
	halfAngleUp_(halfAngleUp)
{
}

ColliderCone::~ColliderCone(void)
{
}

VECTOR ColliderCone::CalcRightAxis(const VECTOR& axis) const
{
	VECTOR right = follow_->GetRight();
	right = VSub(right, VScale(axis, VDot(right, axis)));
	if (VDot(right, right) <= HIT_EPSILON)
	{
		right = follow_->GetUp();
		right = VSub(right, VScale(axis, VDot(right, axis)));
	}
	return VNorm(right);
}

bool ColliderCone::IsHitPoint(const VECTOR& point) const
{
	return IsHitExpandedPoint(point, 0.0f);
}

bool ColliderCone::IsHitCapsule(const ColliderCapsule* colliderCapsule) const
{
	if (colliderCapsule == nullptr)
	{
		return false;
	}

	float radius = colliderCapsule->GetRadius();
	for (int i = 0; i <= CAPSULE_SAMPLE_COUNT; i++)
	{
		float rate =
			static_cast<float>(i) / static_cast<float>(CAPSULE_SAMPLE_COUNT);
		if (IsHitExpandedPoint(colliderCapsule->GetPosOnLine(rate), radius))
		{
			return true;
		}
	}
	return false;
}

void ColliderCone::DrawDebug(int color)
{
	VECTOR apex = GetRotPos(localPosApex_);
	VECTOR axis = VNorm(follow_->quaRot.PosAxis(localAxis_));
	VECTOR right = CalcRightAxis(axis);
	VECTOR up = VNorm(VCross(axis, right));
	VECTOR baseCenter = VAdd(apex, VScale(axis, height_));
	float baseRadiusRight = height_ * tanf(halfAngleRight_);
	float baseRadiusUp = height_ * tanf(halfAngleUp_);
	VECTOR previous = VAdd(baseCenter, VScale(right, baseRadiusRight));

	for (int i = 1; i <= DRAW_DIV_NUM; i++)
	{
		float rad = DX_PI_F * 2.0f *
			static_cast<float>(i) / static_cast<float>(DRAW_DIV_NUM);
		VECTOR offset = VAdd(
			VScale(right, cosf(rad) * baseRadiusRight),
			VScale(up, sinf(rad) * baseRadiusUp));
		VECTOR current = VAdd(baseCenter, offset);

		DrawLine3D(previous, current, color);
		DrawLine3D(apex, current, color);
		previous = current;
	}
}

bool ColliderCone::IsHitExpandedPoint(
	const VECTOR& point, float expandRadius) const
{
	VECTOR apex = GetRotPos(localPosApex_);
	VECTOR axis = VNorm(follow_->quaRot.PosAxis(localAxis_));
	VECTOR right = CalcRightAxis(axis);
	VECTOR up = VNorm(VCross(axis, right));
	VECTOR toPoint = VSub(point, apex);
	float axialDistance = VDot(toPoint, axis);
	if (axialDistance < -expandRadius ||
		axialDistance > height_ + expandRadius)
	{
		return false;
	}

	float coneAxialDistance = axialDistance;
	if (coneAxialDistance < 0.0f)
	{
		coneAxialDistance = 0.0f;
	}
	else if (coneAxialDistance > height_)
	{
		coneAxialDistance = height_;
	}

	float rightDistance = VDot(toPoint, right);
	float upDistance = VDot(toPoint, up);
	float radiusRight =
		coneAxialDistance * tanf(halfAngleRight_) + expandRadius;
	float radiusUp =
		coneAxialDistance * tanf(halfAngleUp_) + expandRadius;

	if (radiusRight <= HIT_EPSILON || radiusUp <= HIT_EPSILON)
	{
		return rightDistance * rightDistance +
			upDistance * upDistance <= HIT_EPSILON;
	}

	float ellipseRate =
		(rightDistance * rightDistance) / (radiusRight * radiusRight) +
		(upDistance * upDistance) / (radiusUp * radiusUp);
	return ellipseRate <= 1.0f + HIT_EPSILON;
}