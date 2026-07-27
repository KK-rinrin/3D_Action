#pragma once
#include <DxLib.h>
#include "ColliderBase.h"

class Transform;
class ColliderCapsule;

// XZ平面上の円形コライダ
class ColliderCircle : public ColliderBase
{
public:
	ColliderCircle(
		TAG tag, const Transform* follow, const VECTOR& localPos, float radius);
	~ColliderCircle(void);

	VECTOR GetPos(void) const;
	float GetRadius(void) const;

	// XZ平面上でカプセルとの衝突を判定する
	bool IsHit(const ColliderCapsule* collider) const override;

	// ステージモデルとの押し戻しには使用しない
	VECTOR GetPosPushBackAlongNormal(
		const MV1_COLL_RESULT_POLY& hitColPoly,
		int maxTryCnt,
		float pushDistance) const override;

protected:
	void DrawDebug(int color) override;

private:
	VECTOR localPos_;
	float radius_;
};
