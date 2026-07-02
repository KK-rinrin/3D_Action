#include "ColliderSphere.h"
#include "../Common/Transform.h"
#include "ColliderModel.h"
#include "ColliderCapsule.h"

ColliderSphere::ColliderSphere(
	TAG tag, const Transform* follow, const VECTOR& localPos, float radius)
	: ColliderBase(SHAPE::SPHERE, tag, follow)
	, localPos_(localPos)
	, radius_(radius)
{

}

ColliderSphere::~ColliderSphere()
{

}

const VECTOR& ColliderSphere::GetLocalPos(void) const
{
	return localPos_;
}

void ColliderSphere::SetLocalPos(const VECTOR& localPos)
{
	localPos_ = localPos;
}

VECTOR ColliderSphere::GetPos(void) const
{
	return  GetRotPos(localPos_);
}

float ColliderSphere::GetRadius() const
{
	return radius_;
}

void ColliderSphere::SetRadius(float radius)
{
	radius_ = radius;
}

VECTOR ColliderSphere::GetPosPushBackAlongNormal(
	const MV1_COLL_RESULT_POLY& hitColPoly,
	int maxTryCnt,
	float pushDistance) const
{
	// コピー生成
	Transform tmpTransform = *follow_;
	ColliderSphere tmpSphere = *this;
	tmpSphere.SetFollow(&tmpTransform);
	// 衝突補正処理
	int sphereCnt = 0;
	while (sphereCnt < maxTryCnt)
	{
		// 球体と三角形の当たり判定
		if (HitCheck_Sphere_Triangle(tmpSphere.GetPos(), tmpSphere.GetRadius(),
			hitColPoly.Position[0], hitColPoly.Position[1], hitColPoly.Position[2]))
		{
			// 衝突していたら法線方向に押し戻し
			tmpTransform.pos =
				VAdd(hitColPoly.HitPosition, VScale(hitColPoly.Normal, pushDistance));

		}
		sphereCnt++;
	}
	return tmpTransform.pos;
}

bool ColliderSphere::IsHitPoint(const VECTOR& pos) const
{
	VECTOR center = GetPos();
	VECTOR diff = VSub(pos, center);
	float dist2 = VDot(diff, diff);
	return dist2 <= radius_ * radius_;
}

bool ColliderSphere::IsHit(const ColliderSphere* collider) const
{
	if (collider == nullptr) return false;

	auto& temp = collider;

	return HitCheck_Sphere_Sphere(GetPos(), radius_,
		temp->GetPos(), temp->GetRadius());
}

bool ColliderSphere::IsHit(const ColliderCapsule* collider) const
{
	if (collider == nullptr) return false;

	auto& temp = collider;

	return HitCheck_Sphere_Capsule(GetPos(), radius_,
		temp->GetPosTop(), temp->GetPosDown(), temp->GetRadius());
}


void ColliderSphere::DrawDebug(int color)
{
	DrawSphere3D(GetPos(), radius_, 5, color, color, false);
}