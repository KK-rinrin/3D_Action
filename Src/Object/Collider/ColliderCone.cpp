#include <cmath>
#include "../Common/Transform.h"
#include "ColliderCapsule.h"
#include "ColliderCone.h"

// コンストラクタ
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

// デストラクタ
ColliderCone::~ColliderCone(void)
{
}

// 軸から右方向ベクトルを計算する
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

// 点との当たり判定（拡張半径なし）を行う
bool ColliderCone::IsHitPoint(const VECTOR& point) const
{
	return IsHitExpandedPoint(point, 0.0f);
}

// カプセルとの当たり判定を行う
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

// デバッグ描画を行う
void ColliderCone::DrawDebug(int color)
{
	// 頂点位置（ワールド座標）
	VECTOR apex = GetRotPos(localPosApex_);
	// 軸方向（正規化）
	VECTOR axis = VNorm(follow_->quaRot.PosAxis(localAxis_));
	// 右方向ベクトル
	VECTOR right = CalcRightAxis(axis);
	// 上方向ベクトル
	VECTOR up = VNorm(VCross(axis, right));
	// 底面中心のワールド座標
	VECTOR baseCenter = VAdd(apex, VScale(axis, height_));
	// 底面の半径（右方向）
	float baseRadiusRight = height_ * tanf(halfAngleRight_);
	// 底面の半径（上方向）
	float baseRadiusUp = height_ * tanf(halfAngleUp_);
	// 前回の頂点位置
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

// 点を拡張半径で判定する内部処理
bool ColliderCone::IsHitExpandedPoint(
	const VECTOR& point, float expandRadius) const
{
	// 頂点位置（ワールド座標）
	VECTOR apex = GetRotPos(localPosApex_);
	// 軸方向（正規化）
	VECTOR axis = VNorm(follow_->quaRot.PosAxis(localAxis_));
	// 右方向ベクトル
	VECTOR right = CalcRightAxis(axis);
	// 上方向ベクトル
	VECTOR up = VNorm(VCross(axis, right));
	// 頂点から点へのベクトル
	VECTOR toPoint = VSub(point, apex);
	// 軸方向への距離
	float axialDistance = VDot(toPoint, axis);
	// 軸方向の範囲外なら衝突なし
	if (axialDistance < -expandRadius ||
		axialDistance > height_ + expandRadius)
	{
		return false;
	}

	// 円錐内部での軸方向距離を0..heightにクランプ
	float coneAxialDistance = axialDistance;
	if (coneAxialDistance < 0.0f)
	{
		coneAxialDistance = 0.0f;
	}
	else if (coneAxialDistance > height_)
	{
		coneAxialDistance = height_;
	}

	// 右方向と上方向への距離
	float rightDistance = VDot(toPoint, right);
	float upDistance = VDot(toPoint, up);
	// 右方向と上方向の半径計算
	float radiusRight =
		coneAxialDistance * tanf(halfAngleRight_) + expandRadius;
	float radiusUp =
		coneAxialDistance * tanf(halfAngleUp_) + expandRadius;

	if (radiusRight <= HIT_EPSILON || radiusUp <= HIT_EPSILON)
	{
		return rightDistance * rightDistance +
			upDistance * upDistance <= HIT_EPSILON;
	}

	// 楕円判定。1以内なら内部
	float ellipseRate =
		(rightDistance * rightDistance) / (radiusRight * radiusRight) +
		(upDistance * upDistance) / (radiusUp * radiusUp);
	return ellipseRate <= 1.0f + HIT_EPSILON;
}
