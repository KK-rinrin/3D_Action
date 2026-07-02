#include "../../../Utility/SchoolUtility.h"
#include "../../../Utility/ModelFrameUtility.h"
#include "../../Common/Transform.h"
#include "WeaponBase.h"
WeaponBase::WeaponBase(const Transform & followTransform, int followFrameId)
	:
	followTransform_(followTransform),
	followFrameId_(followFrameId),
	localPos_(SchoolUtility::VECTOR_ZERO),
	localRot_(SchoolUtility::VECTOR_ZERO)
{
}

WeaponBase::~WeaponBase(void)
{
}

void WeaponBase::Update(void)
{
	// 対象フレームの位置にtargetを配置し、
	// 対象フレームの回転に加え、指定した相対座標・回転を加える
	ModelFrameUtility::SetFrameWorldMatrix(
		followTransform_, followFrameId_, transform_,
		localPos_, localRot_
	);

	// 上記関数内で更新された行列情報からクォータニオンに変換
	transform_.quaRot = Quaternion::GetRotation(transform_.matRot);

	// 大きさ、回転(クォータニオン)、座標を元にモデルを更新
	transform_.Update();
}