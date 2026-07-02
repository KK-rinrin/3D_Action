#pragma once
#include <DxLib.h>
#include "../ActorBase.h"
class Transform;

class WeaponBase : public ActorBase
{
public:
	enum class COLLIDER_TYPE
	{
		CAPSULE,
	};

	// コンストラクタ
	WeaponBase(const Transform& followTransform, int followFrameId);

	// デストラクタ
	virtual ~WeaponBase(void) override;

	// 更新
	virtual void Update(void) override;

protected:
	// 追従先Transform
	const Transform& followTransform_;

	// 追従対象のフレームID
	int followFrameId_;

	// ローカル座標
	VECTOR localPos_;

	// ローカル回転
	VECTOR localRot_;
};