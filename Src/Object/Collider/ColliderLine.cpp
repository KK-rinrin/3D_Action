#include "../Common/Transform.h"
#include "ColliderLine.h"
#include "ColliderModel.h"
#include "../../Utility/SchoolUtility.h"
ColliderLine::ColliderLine(
	TAG tag, const Transform* follow,
	const VECTOR& localPosStart, const VECTOR& localPosEnd)
	:
	ColliderBase(SHAPE::LINE, tag, follow),
	localPosStart_(localPosStart),
	localPosEnd_(localPosEnd)
{
}
ColliderLine::~ColliderLine(void)
{
}
void ColliderLine::SetLocalPosStart(const VECTOR& pos)
{
	localPosStart_ = pos;
}
void ColliderLine::SetLocalPosEnd(const VECTOR& pos)
{
	localPosEnd_ = pos;
}
const VECTOR& ColliderLine::GetLocalPosStart(void) const
{
	return localPosStart_;
}
const VECTOR& ColliderLine::GetLocalPosEnd(void) const
{
	return localPosEnd_;
}
VECTOR ColliderLine::GetPosStart(void) const
{
	return GetRotPos(localPosStart_);
}
VECTOR ColliderLine::GetPosEnd(void) const
{
	return GetRotPos(localPosEnd_);
}
bool ColliderLine::PushBack(const ColliderModel* colliderModel, Transform& transform,
	float pushDistance, VECTOR pushDir, bool isExclude, bool isTarget)
{
	bool isHit = false;
	// ステージモデル(地面)との衝突
	auto hits = MV1CollCheck_LineDim(
		colliderModel->GetFollow()->modelId, -1, GetPosStart(), GetPosEnd());

	for (int i = 0; i < hits.HitNum; i++)
	{
		auto hit = hits.Dim[i];
		// 除外フレームは無視する
		if (isExclude && colliderModel->IsExcludeFrame(hit.FrameIndex)) continue;

		// 対象フレーム以外は無視する
		if (isTarget && !colliderModel->IsTargetFrame(hit.FrameIndex)) continue;
		
		// 衝突地点から移動
		//if (transform.pos.y < hit.HitPosition.y)
		// 条件はtransformを用いて衝突物より内側にいるかどうかで判定する
		
		if (VNorm(VSub(transform.pos, hit.HitPosition)).y < pushDir.y)
		{
			// 衝突物より内側にいる場合のみ、位置を修正する
			transform.pos =
				VAdd(hit.HitPosition, VScale(VNorm(pushDir), pushDistance));
		}
		isHit = true;

	}
	// 検出した地面ポリゴン情報の後始末
	MV1CollResultPolyDimTerminate(hits);
	return isHit;
}
void ColliderLine::DrawDebug(int color)
{
	VECTOR s = GetPosStart();
	VECTOR e = GetPosEnd();
	// 線分を描画
	DrawLine3D(s, e, color);
	// 始点・終点を球体で補助表示
	DrawSphere3D(s, RADIUS, DIV_NUM, color, color, true);
	DrawSphere3D(e, RADIUS, DIV_NUM, color, color, true);
}