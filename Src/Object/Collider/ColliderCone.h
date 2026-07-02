#pragma once
#include <DxLib.h>
#include "ColliderBase.h"

class ColliderCapsule;
class Transform;

class ColliderCone : public ColliderBase
{
public:
	ColliderCone(
		TAG tag, const Transform* follow,
		const VECTOR& localPosApex, const VECTOR& localAxis,
		float height, float halfAngleRight, float halfAngleUp);
	~ColliderCone(void) override;

	bool IsHitPoint(const VECTOR& point) const;
	bool IsHitCapsule(const ColliderCapsule* colliderCapsule) const;

	VECTOR GetPosPushBackAlongNormal(
		const MV1_COLL_RESULT_POLY& hitColPoly,
		int maxTryCnt,
		float pushDistance) const override
	{
		return {};
	}

protected:
	void DrawDebug(int color) override;

private:
	static constexpr int DRAW_DIV_NUM = 16;
	static constexpr int CAPSULE_SAMPLE_COUNT = 32;
	static constexpr float HIT_EPSILON = 0.0001f;

	VECTOR localPosApex_;
	VECTOR localAxis_;
	float height_;
	float halfAngleRight_;
	float halfAngleUp_;

	VECTOR CalcRightAxis(const VECTOR& axis) const;
	bool IsHitExpandedPoint(const VECTOR& point, float expandRadius) const;
};
