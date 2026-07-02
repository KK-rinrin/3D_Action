#include "ColliderModel.h"
#include "../Common/Transform.h"
#include "../../Utility/SchoolUtility.h"
ColliderModel::ColliderModel(TAG tag, const Transform* follow)
	:
	ColliderBase(SHAPE::MODEL, tag, follow)
{
}
ColliderModel::~ColliderModel(void)
{
}

void ColliderModel::AddExcludeFrameIds(const std::string& name)
{
	// フレーム数を取得
	int num = MV1GetFrameNum(follow_->modelId);
	for (int i = 0; i < num; i++)
	{
		// フレーム名称を取得
		std::string frameName = MV1GetFrameName(follow_->modelId, i);
		if (frameName.find(name) != std::string::npos)
		{
			// 除外フレームに追加
			excludeFrameIds_.emplace_back(i);
		}
	}
}
void ColliderModel::ClearExcludeFrame(void)
{
	excludeFrameIds_.clear();
}
bool ColliderModel::IsExcludeFrame(int frameIdx) const
{
	// 除外判定
	if (std::find(
		excludeFrameIds_.begin(),
		excludeFrameIds_.end(),
		frameIdx) != excludeFrameIds_.end())
	{
		// 除外に該当する
		return true;
	}
	return false;
}

void ColliderModel::AddTargetFrameIds(const std::string& name)
{
	// フレーム数を取得
	int num = MV1GetFrameNum(follow_->modelId);
	for (int i = 0; i < num; i++)
	{
		// フレーム名称を取得
		std::string frameName = MV1GetFrameName(follow_->modelId, i);
		if (frameName.find(name) != std::string::npos)
		{
			// 除外フレームに追加
			targetFrameIds_.emplace_back(i);
		}
	}
}

void ColliderModel::ClearTargetFrame(void)
{
	targetFrameIds_.clear();
}

bool ColliderModel::IsTargetFrame(int frameIdx) const
{
	// 除外判定
	if (std::find(
		targetFrameIds_.begin(),
		targetFrameIds_.end(),
		frameIdx) != targetFrameIds_.end())
	{
		// 除外に該当する
		return true;
	}
	return false;
}

MV1_COLL_RESULT_POLY ColliderModel::GetNearestHitPolyLine(const VECTOR& start, const VECTOR& end, bool isExclude, bool isTarget) const
{
	// コピー生成
	Transform tmpTransform = *follow_;

	// 線分で衝突判定
	auto hits = MV1CollCheck_LineDim(
		tmpTransform.modelId,
		-1,
		start,
		end
	);

	// 追従対象に一番近い衝突点を探す
	MV1_COLL_RESULT_POLY hitPoly = {};
	double minDist = DBL_MAX;
	for (int i = 0; i < hits.HitNum; i++)
	{
		const auto& hit = hits.Dim[i];

		// 除外フレームは無視する(isExclude == true)
		if (isExclude && IsExcludeFrame(hit.FrameIndex))
		{
			continue;
		}

		// 対象フレーム以外は無視する(isTarget == true)
		if (isTarget && !IsTargetFrame(hit.FrameIndex))
		{
			continue;
		}

		//// 衝突判定
		//hitPoly.HitFlag = true;
		
		// 距離判定
		int dist = SchoolUtility::Distance(start, tmpTransform.pos);
		if (dist < minDist)
		{
			// 追従対象に一番近い衝突点を優先
			minDist = dist;
			hitPoly = hit;
		}
	}

	// 検出した地面ポリゴン情報の後始末
	MV1CollResultPolyDimTerminate(hits);
	
	return hitPoly;
}
