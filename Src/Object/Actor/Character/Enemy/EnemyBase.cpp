#include "EnemyBase.h"
#include "../../../../Utility/SchoolUtility.h"
#include "../../../Collider/ColliderSphere.h"
#include "../../../Collider/ColliderCapsule.h"
#include "../../../Collider/ColliderModel.h"
#include "../../../Common/AnimationController.h"
#include "../../../../Manager/SceneManager.h"

EnemyBase::EnemyBase(const EnemyBase::EnemyData& data)
	:
	type_(data.type),
	hp_(data.hp),
	stateBase_(-1),
	defaultPos_(data.defaultPos),
	moveRadius_(data.radius),
	isHit_(false),
	isVisible_(true),
	targetTransform_(nullptr),
	obstacleStuckStep_(0.0f),
	obstacleAvoidStep_(0.0f),
	obstacleAvoidSide_(1.0f),
	obstacleMoveDir_(SchoolUtility::VECTOR_ZERO),
	isObstacleMoveCheck_(false),
	canObstacleAvoid_(false),
	isObstacleStuck_(false),
	damagedStep_(0.0f)
{
	// 初期座標の設定
	transform_.pos = defaultPos_;
}

EnemyBase::~EnemyBase(void)
{
}

void EnemyBase::SetTargetTransform(const Transform* targetTransform)
{
	targetTransform_ = targetTransform;
}

void EnemyBase::Draw(void)
{
	if (!isVisible_) return;

	CharacterBase::Draw();

#ifdef _DEBUG

	// 移動可能範囲のデバッグ描画
	DrawSphere3D(defaultPos_, moveRadius_, 16, 0x000099, 0x000099, false);

#endif // _DEBUG
}

void EnemyBase::InitPost(void)
{
	// 点滅用初期処理
	InitBlink();
}

void EnemyBase::CollisionReserve(void)
{
	// この処理では、横移動を適用する前に移動予定地点の地面を調べ、
	// 上り坂や下り坂に合わせるためのY方向移動量を先に用意する

	// ノックバックなどY方向の移動中は地面へ追従させない
	if (fabsf(movePow_.y) > SchoolUtility::kEpsilonNormalSqrt) return;

	// 地面を探す方向として、移動量からXZ成分だけを取り出す
	VECTOR moveXZ = movePow_;
	moveXZ.y = 0.0f;

	// 待機中など横移動していない場合は前方を調べる必要がない
	if (SchoolUtility::SqrMagnitude(moveXZ)
		<= SchoolUtility::kEpsilonNormalSqrt)
	{
		return;
	}

	// 現在座標に今回の横移動量を加え、移動予定地点を求める
	VECTOR probePos = VAdd(transform_.pos, moveXZ);

	// 上り坂や少し高い段差も線分に含められるよう、
	// 移動予定地点より上に探索線分の始点を置く
	VECTOR probeStart = probePos;
	probeStart.y += GROUND_FOLLOW_MAX_STEP_HEIGHT
		+ GROUND_FOLLOW_PROBE_MARGIN;

	// 下り坂でも地面を見失わないよう、
	// 移動予定地点より下に探索線分の終点を置く
	VECTOR probeEnd = probePos;
	probeEnd.y -= GROUND_FOLLOW_MAX_SNAP_DOWN
		+ GROUND_FOLLOW_PROBE_MARGIN;

	// 複数のステージモデルを調べた結果から、使用する地面を保持する
	bool isFoundGround = false;
	MV1_COLL_RESULT_POLY groundHit = {};
	for (const auto& hitCol : hitColliders_)
	{
		// 無効なコライダとステージ以外のコライダは調べない
		if (hitCol == nullptr || !hitCol->IsValid()) continue;
		if (hitCol->GetTag() != ColliderBase::TAG::STAGE) continue;

		// 地形ポリゴンとの線分判定ができるモデルコライダだけを使う
		if (hitCol->GetShape() != ColliderBase::SHAPE::MODEL) continue;

		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(hitCol);
		if (colliderModel == nullptr) continue;

		// 探索線分に当たった中から、始点に一番近いポリゴンを取得する
		MV1_COLL_RESULT_POLY hit = colliderModel->GetNearestHitPolyLine(
			probeStart, probeEnd, true, false);

		// 地面が見つからなければ次のステージモデルを調べる
		if (!hit.HitFlag) continue;

		// 法線のY成分が小さい急斜面や壁は歩行可能な地面として扱わない
		if (hit.Normal.y < ColliderCapsule::MIN_WALKABLE_GROUND_NORMAL_Y)
		{
			continue;
		}

		// 複数の地面が重なる場合は一番上の面を使う
		if (!isFoundGround || hit.HitPosition.y > groundHit.HitPosition.y)
		{
			isFoundGround = true;
			groundHit = hit;
		}
	}

	// 移動予定地点に歩行可能な地面がなければY移動量を変更しない
	if (!isFoundGround) return;

	// 重力判定と同じ押し戻し距離だけ地面から浮かせた目標Y座標を作る
	const float targetY = groundHit.HitPosition.y + COLLISION_BACK_DIS;
	// 現在のY座標から目標Y座標までに必要な移動量を求める
	const float heightDiff = targetY - transform_.pos.y;

	// 一度に上れる高さ、または下へ吸着できる高さを超える場合は追従しない
	if (heightDiff > GROUND_FOLLOW_MAX_STEP_HEIGHT
		|| heightDiff < -GROUND_FOLLOW_MAX_SNAP_DOWN)
	{
		return;
	}

	// 横移動と同時に前方の地面の高さへ合わせる
	movePow_.y = heightDiff;
}

void EnemyBase::CollisionPost(void)
{
	// 障害物への詰まりと迂回を更新
	UpdateObstacleAvoidance();

	// 衝突判定(プレイヤーの武器)
	CollisionWeapon();
}

bool EnemyBase::InMovableRange(void) const
{
	bool ret = false;

	// 初期位置からのXZ平面上の距離
	VECTOR defaultPos = defaultPos_;
	VECTOR pos = transform_.pos;
	defaultPos.y = 0.0f;
	pos.y = 0.0f;
	float dis = static_cast<float>(
		SchoolUtility::SqrMagnitude(defaultPos, pos));

	// 指定距離判定
	if (dis < moveRadius_ * moveRadius_)
	{
		return true;
	}

	return ret;
}

VECTOR EnemyBase::GetObstacleAvoidMoveDir(const VECTOR& targetDir)
{
	VECTOR dir = targetDir;
	dir.y = 0.0f;
	if (SchoolUtility::SqrMagnitude(dir) <= SchoolUtility::kEpsilonNormalSqrt)
	{
		return SchoolUtility::VECTOR_ZERO;
	}

	dir = VNorm(dir);

	if (obstacleAvoidStep_ > 0.0f)
	{
		// 正面へ進む成分を捨て、選んだ側の真横へ移動する
		VECTOR sideDir = VGet(dir.z, 0.0f, -dir.x);
		dir = VScale(sideDir, obstacleAvoidSide_);
	}

	obstacleMoveDir_ = dir;
	isObstacleMoveCheck_ = true;
	canObstacleAvoid_ = true;
	return dir;
}

void EnemyBase::ReserveObstacleStuckCheck(const VECTOR& moveDir)
{
	// 高低差を無視して、XZ平面上の移動方向だけを判定に使う
	obstacleMoveDir_ = moveDir;
	obstacleMoveDir_.y = 0.0f;

	// 移動方向がなければ詰まり判定を行わない
	if (SchoolUtility::SqrMagnitude(obstacleMoveDir_)
		<= SchoolUtility::kEpsilonNormalSqrt)
	{
		return;
	}
	obstacleMoveDir_ = VNorm(obstacleMoveDir_);

	// 衝突処理後に、指定方向へ実際に進めたかを確認する
	isObstacleMoveCheck_ = true;

	// 徘徊では迂回せず、詰まったことだけを派生クラスへ通知する
	canObstacleAvoid_ = false;
}

bool EnemyBase::ConsumeObstacleStuck(void)
{
	if (!isObstacleStuck_) return false;

	isObstacleStuck_ = false;
	return true;
}

void EnemyBase::ChangeState(int state)
{
	// 前の行動で残った詰まり判定や迂回を持ち越さない
	obstacleStuckStep_ = 0.0f;
	obstacleAvoidStep_ = 0.0f;
	obstacleMoveDir_ = SchoolUtility::VECTOR_ZERO;
	isObstacleMoveCheck_ = false;
	canObstacleAvoid_ = false;
	isObstacleStuck_ = false;

	stateBase_ = state;

	// 各状態遷移の初期処理
	stateChanges_[stateBase_]();
}

bool EnemyBase::InSearch(float viewRange, float viewAngle)
{
	return false;
}

void EnemyBase::Hide(void)
{
	isVisible_ = false;
	movePow_ = SchoolUtility::VECTOR_ZERO;
	jumpPow_ = SchoolUtility::VECTOR_ZERO;
	transform_.scl = SchoolUtility::VECTOR_ZERO;
	transform_.Update();
	SetAllColliderValid(false);
	ClearHitCollider();
}

void EnemyBase::CollisionWeapon(void)
{
	if (isHit_ || IsInValidDamage()) return;

	for (const auto& hitCol : hitColliders_)
	{
		// 有効でなければ処理を飛ばす
		if (!hitCol->IsValid()) continue;

		// プレイヤー武器以外は処理を飛ばす
		if (hitCol->GetTag() != ColliderBase::TAG::PLAYER_WEAPON) continue;

		// 派生クラスへキャスト
		const ColliderCapsule* weaponCol =
			dynamic_cast<const ColliderCapsule*>(hitCol);

		if (weaponCol == nullptr) continue;

		ColliderCapsule* colMyCap = dynamic_cast<ColliderCapsule*>
			(ownColliders_.at(static_cast<int>(COLLIDER_TYPE::CAPSULE)));

		if (weaponCol->IsHit(colMyCap))
		{
			const bool isKnockBack = weaponCol->GetKnockBackPow() > 0.0f;
			if (isKnockBack)
			{
				VECTOR diff = VSub(
					weaponCol->GetCenter(), colMyCap->GetCenter());

				knockBackParam_.Init(
					diff, 3000.0f * weaponCol->GetKnockBackPow());

				OnStartKnockBack();
			}
			Damage(1);
			if (!isKnockBack)
			{
				StartDamaged();
			}
		}
	}
}

void EnemyBase::UpdateObstacleAvoidance(void)
{
	// 迂回開始時に決めた側へ、指定時間だけ進み続ける
	const bool isAvoiding = obstacleAvoidStep_ > 0.0f;
	if (obstacleAvoidStep_ > 0.0f)
	{
		obstacleAvoidStep_ -= scnMng_.GetDeltaTime();
		if (obstacleAvoidStep_ < 0.0f)
		{
			obstacleAvoidStep_ = 0.0f;
		}
	}

	// 迂回中に再判定すると短時間で左右が入れ替わるため、
	// 迂回が終わるまでは詰まり時間を数え直さない
	if (isAvoiding)
	{
		obstacleStuckStep_ = 0.0f;
		isObstacleMoveCheck_ = false;
		canObstacleAvoid_ = false;
		return;
	}

	if (!isObstacleMoveCheck_)
	{
		obstacleStuckStep_ = 0.0f;
		canObstacleAvoid_ = false;
		return;
	}

	VECTOR moved = VSub(transform_.pos, prevPos_);
	moved.y = 0.0f;
	const float forwardDistance = VDot(moved, obstacleMoveDir_);

	if (forwardDistance <= OBSTACLE_MOVE_DISTANCE)
	{
		obstacleStuckStep_ += scnMng_.GetDeltaTime();
		if (obstacleStuckStep_ >= OBSTACLE_STUCK_TIME)
		{
			obstacleStuckStep_ = 0.0f;

			if (canObstacleAvoid_)
			{
				// 同じ側で再び詰まった場合も抜けられるよう左右を交互に試す
				obstacleAvoidSide_ *= -1.0f;
				obstacleAvoidStep_ = OBSTACLE_AVOID_TIME;
			}
			else
			{
				isObstacleStuck_ = true;
			}
		}
	}
	else
	{
		obstacleStuckStep_ = 0.0f;
	}

	isObstacleMoveCheck_ = false;
	canObstacleAvoid_ = false;
}

void EnemyBase::UpdateKnockBack(void)
{
	// 移動量の計算
	movePow_ = knockBackParam_.GetMovePow();

	// 力の減衰
	knockBackParam_.Decay();

	// 点滅用時間経過
	knockBackParam_.step += scnMng_.GetDeltaTime();

	// 点滅
	Blink(knockBackParam_.step);

	// ノックバック終了判定
	if (knockBackParam_.IsEnd())
	{
		OnEndKnockBack();

		// 点滅終了処理
		SetDefaultEmiColor();
	}
}

void EnemyBase::StartDamaged(void)
{
	damagedStep_ = 0.0f;
	OnStartDamaged();
}

void EnemyBase::UpdateDamaged(void)
{
	damagedStep_ += scnMng_.GetDeltaTime();
	Blink(damagedStep_);

	if (animController_->IsEnd(animTypeDamaged_))
	{
		OnEndDamaged();
		SetDefaultEmiColor();
	}
}

void EnemyBase::InitBlink(void)
{
	// 自己発光の強さ
	const COLOR_F EMI_POWER = { 1.0f, 0.3f, 0.3f, 0.0f };

	// マテリアルごとの初期自己発光を保存しておく
	int num = MV1GetMaterialNum(transform_.modelId);

	for (int i = 0; i < num; i++)
	{
		// 初期自己発光
		COLOR_F dif = MV1GetMaterialEmiColor(transform_.modelId, i);
		materialEmiColors_.emplace_back(dif);

		// 点滅時自己発光
		dif.r += EMI_POWER.r;
		if (dif.r > 1.0f) { dif.r = 1.0f; }

		dif.g += EMI_POWER.g;
		if (dif.g > 1.0f) { dif.g = 1.0f; }

		dif.b += EMI_POWER.b;
		if (dif.b > 1.0f) { dif.b = 1.0f; }

		materialEmiBlinkColors_.emplace_back(dif);
	}
}

void EnemyBase::Blink(float step)
{
	// 点滅間隔
	constexpr int TERM_BLINK = 5;

	// 点滅スピード
	constexpr float SPEED_BLINK = 20.0f;

	int intStep = static_cast<int>(step * SPEED_BLINK);

	// モデルの点滅処理
	int i = 0;
	if (intStep % TERM_BLINK <= TERM_BLINK / 5)
	{
		// デフォルトの自己発光色
		for (const auto& color : materialEmiColors_)
		{
			MV1SetMaterialEmiColor(transform_.modelId, i++, color);
		}
	}
	else
	{
		// 点滅時の自己発光色
		for (const auto& color : materialEmiBlinkColors_)
		{
			MV1SetMaterialEmiColor(transform_.modelId, i++, color);
		}
	}
}

void EnemyBase::SetDefaultEmiColor(void)
{
	int i = 0;
	for (const auto& color : materialEmiColors_)
	{
		MV1SetMaterialEmiColor(transform_.modelId, i++, color);
	}
}

void EnemyBase::Damage(int damage)
{
	hp_ -= damage;
	if (hp_ < 0) hp_ = 0;
}
