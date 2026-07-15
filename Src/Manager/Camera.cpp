#include <DxLib.h>
#include <EffekseerForDXLib.h>
#include "../Utility/SchoolUtility.h"
#include "../Manager/InputBinder.h"
#include "../Object/Common/Transform.h"
#include "Camera.h"
#include "../Object/Collider/ColliderModel.h"
#include "../Object/Collider/ColliderSphere.h"

Camera::Camera(InputBinder* inputBinder)
	:
	followTransform_(nullptr),
	mode_(MODE::NONE),
	angles_(SchoolUtility::VECTOR_ZERO),
	rotY_(Quaternion::Identity()),
	targetPos_(SchoolUtility::VECTOR_ZERO),
	prePos_(SchoolUtility::VECTOR_ZERO),
	inputBinder_(inputBinder)
{
	// DxLibの初期設定では、
	// カメラの位置が x = 320.0f, y = 240.0f, z = (画面のサイズによって変化)、
	// 注視点の位置は x = 320.0f, y = 240.0f, z = 1.0f
	// カメラの上方向は x = 0.0f, y = 1.0f, z = 0.0f
	// 右上位置からZ軸のプラス方向を見るようなカメラ
}

Camera::~Camera(void)
{
}

void Camera::Update(void)
{
}

void Camera::SetBeforeDraw(void)
{
	// クリップ距離を設定する(SetDrawScreenでリセットされる)
	SetCameraNearFar(VIEW_NEAR, VIEW_FAR);

	// 更新前情報
	prePos_ = transform_.pos;

	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		SetBeforeDrawFixedPoint();
		break;
	case Camera::MODE::FREE:
		SetBeforeDrawFree();
		break;
	case Camera::MODE::FOLLOW:
		SetBeforeDrawFollow();
		break;
	}

	// カメラの設定(位置と注視点による制御)
	SetCameraPositionAndTargetAndUpVec(
		transform_.pos, 
		targetPos_, 
		transform_.quaRot.GetUp()
	);

	// DXライブラリのカメラとEffekseerのカメラを同期する。
	Effekseer_Sync3DSetting();

}

void Camera::DrawDebug(void)
{
}

void Camera::Release(void)
{
}

void Camera::SetFollow(const Transform* follow)
{
	followTransform_ = follow;
}

const VECTOR& Camera::GetPos(void) const
{
	return transform_.pos;
}

const VECTOR& Camera::GetAngles(void) const
{
	return angles_;
}

const VECTOR& Camera::GetTargetPos(void) const
{
	return targetPos_;
}

const Quaternion& Camera::GetQuaRot(void) const
{
	return transform_.quaRot;
}

const Quaternion& Camera::GetQuaRotY(void) const
{
	return rotY_;
}

VECTOR Camera::GetForward(void) const
{
	return VNorm(VSub(targetPos_, transform_.pos));
}

void Camera::ChangeMode(MODE mode)
{

	// カメラの初期設定
	SetDefault();

	// カメラモードの変更
	mode_ = mode;

	// 変更時の初期化処理
	switch (mode_)
	{
	case Camera::MODE::FIXED_POINT:
		SetBeforeDrawFixedPoint();
		break;
	case Camera::MODE::FREE:
		SetBeforeDrawFree();
		break;
	case Camera::MODE::FOLLOW:
		SetBeforeDrawFollow();
		break;
	}
}

void Camera::SetDefault(void)
{

	// カメラの初期設定
	transform_.pos = DERFAULT_POS;

	// カメラ角
	angles_ = DERFAULT_ANGLES;
	transform_.quaRot = Quaternion::Identity();

	// 注視点
	targetPos_ = SchoolUtility::VECTOR_ZERO;

}

void Camera::SyncFollow(void)
{

	// 同期先の位置
	VECTOR pos = followTransform_->pos;

	// Y軸
	rotY_ = Quaternion::AngleAxis(angles_.y, SchoolUtility::AXIS_Y);

	// Y軸 + X軸
	transform_.quaRot = rotY_.Mult(Quaternion::AngleAxis(angles_.x, SchoolUtility::AXIS_X));

	VECTOR localPos;

	// 注視点
	localPos = transform_.quaRot.PosAxis(FOLLOW_TARGET_LOCAL_POS);
	targetPos_ = VAdd(pos, localPos);

	// カメラ位置
	localPos = transform_.quaRot.PosAxis(FOLLOW_CAMERA_LOCAL_POS);
	transform_.pos = VAdd(pos, localPos);

}

void Camera::ProcessRot(bool isLimit)
{
	angles_ = inputBinder_->GetRotPowCamera();

	// 角度制限
	if (isLimit && angles_.x < -LIMIT_X_DW_RAD)
	{
		angles_.x = -LIMIT_X_DW_RAD;
	}
	if (isLimit && angles_.x > LIMIT_X_UP_RAD)
	{
		angles_.x = LIMIT_X_UP_RAD;
	}
}

void Camera::ProcessMove(void)
{
	VECTOR moveDir = SchoolUtility::VECTOR_ZERO;

	if (GetJoypadNum() == 0)
	{
		//if (ins.IsNew(KEY_INPUT_W)) { moveDir = AsoUtility::DIR_F; }
		//if (ins.IsNew(KEY_INPUT_S)) { moveDir = AsoUtility::DIR_B; }
		//if (ins.IsNew(KEY_INPUT_A)) { moveDir = AsoUtility::DIR_L; }
		//if (ins.IsNew(KEY_INPUT_D)) { moveDir = AsoUtility::DIR_R; }
	}
	else
	{

		//InputManager::JOYPAD_IN_STATE padState =
		//	ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);

		//// 左スティックの傾き
		//moveDir = ins.GetDirectionXZAKey(padState.AKeyLX, padState.AKeyLY);

	}

	// 移動処理
	if (!SchoolUtility::EqualsVZero(moveDir))
	{

		// 移動させたい方向(ベクトル)に変換

		// 現在の向きからの進行方向を取得
		VECTOR direction = VNorm(transform_.quaRot.PosAxis(moveDir));

		// 移動させたい方向に移動量をかける(=移動量)
		VECTOR movePow = VScale(direction, SPEED);

		// カメラ位置も注視点も移動させる
		transform_.pos = VAdd(transform_.pos, movePow);
		targetPos_ = VAdd(targetPos_, movePow);

	}

}

void Camera::SetBeforeDrawFixedPoint(void)
{
	// 何もしない
}

void Camera::SetBeforeDrawFree(void)
{

	// カメラ操作(回転)
	ProcessRot(false);
	
	// カメラ操作(移動)
	ProcessMove();

	// Y軸
	rotY_ = Quaternion::AngleAxis(angles_.y, SchoolUtility::AXIS_Y);

	// Y軸 + X軸
	transform_.quaRot = rotY_.Mult(Quaternion::AngleAxis(angles_.x, SchoolUtility::AXIS_X));

	// 注視点更新
	targetPos_ = VAdd(transform_.pos, transform_.quaRot.PosAxis(FOLLOW_TARGET_LOCAL_POS));

}

void Camera::SetBeforeDrawFollow(void)
{

	// カメラ操作(回転)
	ProcessRot(true);

	// 追従対象との相対位置を同期
	SyncFollow();

	// 衝突判定
	Collision();

	float y = transform_.pos.y;

	// カメラ位置の補間
	transform_.pos =
		SchoolUtility::Lerp(prePos_, transform_.pos, LERP_RATE_MOVE);

	// y位置だけ補間しない
	transform_.pos.y = y;
}

void Camera::InitLoad()
{

}

void Camera::InitTransform()
{

}

void Camera::InitCollider()
{
	// 主に地面との衝突で使用する球体コライダ
	ColliderSphere* colliderSphere = new ColliderSphere(
		ColliderBase::TAG::CAMERA,
		&transform_,
		SchoolUtility::VECTOR_ZERO,
		COL_CAPSULE_SPHERE
	);
	ownColliders_.emplace(
		static_cast<int>(COLLIDER_TYPE::SPHERE), colliderSphere);
}

void Camera::InitAnimation()
{

}

void Camera::InitPost()
{
	ChangeMode(MODE::FIXED_POINT);

}

void Camera::Collision(void)
{
	// プレイヤーのルートフレーム
	VECTOR start = MV1GetFramePosition(followTransform_->modelId, 1);

	// 毎フレーム最初に全フレームの透明度をリセットする
	for (const auto& collider : hitColliders_)
	{
		if (collider->GetShape() != ColliderBase::SHAPE::MODEL) continue;

		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(collider);
		if (colliderModel == nullptr) continue;

		int modelId = colliderModel->GetFollow()->modelId;
		int frameNum = MV1GetFrameNum(modelId);

		for (int i = 0; i < frameNum; i++)
		{
			MV1SetFrameOpacityRate(modelId, i, 1.0f);
		}

		// 衝突判定（半透明化）
		auto hits = MV1CollCheck_LineDim(
			colliderModel->GetFollow()->modelId,
			-1,
			transform_.pos,
			start
		);
		for (int i = 0; i < hits.HitNum; i++)
		{
			const auto& hit = hits.Dim[i];
			if (!colliderModel->IsTargetFrame(hit.FrameIndex))
				MV1SetFrameOpacityRate(colliderModel->GetFollow()->modelId, hit.FrameIndex, COL_MODEL_ALPHA);
		}

	}

	for (const auto& hitCol : hitColliders_)
	{
		// モデル以外は処理を飛ばす
		if (hitCol->GetShape() != ColliderBase::SHAPE::MODEL) continue;

		// 派生クラスへキャスト
		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(hitCol);

		if (colliderModel == nullptr) continue;

		// 線分とモデルの最近接(startに近い)衝突ポリゴンを取得
		auto hitPoly = colliderModel->GetNearestHitPolyLine(transform_.pos, start, false, true);

		// 衝突していなければ次のコライダへ
		if (!hitPoly.HitFlag) continue;

		// カメラ位置から注視点への方向
		VECTOR dirToTarget = VNorm(VSub(targetPos_, transform_.pos));

		// 衝突点の少し手前にカメラを置く
		transform_.pos = 
			VAdd(hitPoly.HitPosition, VScale(dirToTarget, COLLISION_BACK_DIS));


		// カメラ位置の球体コライダ
		int typeSphere = static_cast<int>(COLLIDER_TYPE::SPHERE);
		// 球体コライダが無ければ処理を抜ける
		if (ownColliders_.count(typeSphere) == 0) continue;
		// 指定された回数と距離で三角形の法線方向に押し戻す
		transform_.pos = ownColliders_.at(typeSphere)->GetPosPushBackAlongNormal(hitPoly,
			CNT_TRY_COLLISION_CAMERA, COLLISION_BACK_DIS);
	}

}