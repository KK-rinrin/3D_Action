#include <cmath>
#include "EnemyRat.h"
#include "../../UI/UISurprise.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../Collider/ColliderLine.h"
#include "../../../Collider/ColliderCapsule.h"
#include "../../../Collider/ColliderModel.h"
#include "../../../Common/AnimationController.h"
#include "../../../../Utility/SchoolUtility.h"
#include "../../../../Manager/SceneManager.h"

EnemyRat::EnemyRat(const EnemyBase::EnemyData& data)
	:
	EnemyBase(data),
	state_(STATE::NONE),
	step_(0.0f),
	moveInRangeTargetPos_(SchoolUtility::VECTOR_ZERO),
	attackIndex_(0),
	attackPhase_(ATTACK_PHASE::PREPARE),
	canStartChase_(true),
	isResumeChase_(false),
	uiSurprise_(nullptr)
{
	knockBackParam_.weight = 50.0f;
}

EnemyRat::~EnemyRat(void)
{
	delete uiSurprise_;
	uiSurprise_ = nullptr;
}

void EnemyRat::Draw(void)
{
	if (!isVisible_) return;

	// 基底クラスの描画処理
	EnemyBase::Draw();

#ifdef _DEBUG
	if (state_ != STATE::DEAD && state_ != STATE::END)
	{
		DrawViewRangeProjection();
	}
#endif // _DEBUG

	if (uiSurprise_ != nullptr)
	{
		uiSurprise_->Draw();
	}
}

void EnemyRat::InitLoad(void)
{
	// 基底クラスのリソースロード
	CharacterBase::InitLoad();

	// モデルの読み込み
	transform_.modelId = MV1DuplicateModel(resMng_.Load(ResourceManager::SRC::ENEMY_RAT).handleId_);

	// 「！」UIのロード
	uiSurprise_ = new UISurprise(
		&transform_, UI_LOCAL_HEIGHT, UI_BOUNCE_HEIGHT, UI_SIZE);
	uiSurprise_->Init();
}

void EnemyRat::InitTransform(void)
{
	// 大きさ、回転、位置の初期化
	transform_.scl = VScale(SchoolUtility::VECTOR_ONE, SCALE);

	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Euler(ROT);

	//transform_.pos = INIT_POS;

	transform_.Update();
}

void EnemyRat::InitCollider(void)
{
	// 線分
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::ENEMY, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::LINE), colLine);

	// カプセル
	ColliderCapsule* colCap = new ColliderCapsule(
		ColliderBase::TAG::ENEMY, &transform_,
		COL_CAPSULE_TOP_LOCAL_POS, COL_CAPSULE_DOWN_LOCAL_POS, COL_CAPSULE_RADIUS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCap);

	// ATTACK用カプセル
	ColliderCapsule* colAttack = new ColliderCapsule(
		ColliderBase::TAG::ENEMY_ATTACK, &transform_,
		COL_ATTACK_TOP_LOCAL_POS, COL_ATTACK_DOWN_LOCAL_POS,
		COL_ATTACK_RADIUS);
	colAttack->SetValid(false);
	AddAttackCollider(colAttack);
}

void EnemyRat::InitAnimation(void)
{
	int type = -1;
	animController_ = new AnimationController(transform_.modelId);

	// IDLEアニメ
	type = static_cast<int>(ANIM_TYPE::IDLE);
	animController_->AddInFbx(type, 30.0f, type);

	// WALKアニメ
	type = static_cast<int>(ANIM_TYPE::WALK);
	animController_->AddInFbx(type, 30.0f, type);

	// ATTACK前動作アニメ
	animController_->AddInFbx(
		ATTACK_PREPARE_ANIM_INDEX, 30.0f,
		ATTACK_PREPARE_ANIM_INDEX);

	// ATTACKアニメ
	animController_->AddInFbx(
		ATTACK_ANIM_INDEX, 30.0f, ATTACK_ANIM_INDEX);

	// HITアニメ
	type = static_cast<int>(ANIM_TYPE::HIT);
	animController_->AddInFbx(type, 20.0f, type);

	// DEADアニメ
	type = static_cast<int>(ANIM_TYPE::DIE);
	animController_->AddInFbx(type, 30.0f, type);

	animController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
}

void EnemyRat::InitPost(void)
{
	EnemyBase::InitPost();

	// 攻撃データ
	ATTACK attack;
	attack.prepareAnimIndex = ATTACK_PREPARE_ANIM_INDEX;
	attack.animIndex = ATTACK_ANIM_INDEX;
	attack.collisionStartRate = 0.25f;
	attack.collisionEndRate = 0.6f;
	attacks_.emplace_back(attack);

	// 状態遷移初期処理登録
	stateChanges_.emplace(static_cast<int>(STATE::NONE),
		std::bind(&EnemyRat::ChangeStateNone, this));

	stateChanges_.emplace(static_cast<int>(STATE::THINK),
		std::bind(&EnemyRat::ChangeStateThink, this));

	stateChanges_.emplace(static_cast<int>(STATE::IDLE),
		std::bind(&EnemyRat::ChangeStateIdle, this));

	stateChanges_.emplace(static_cast<int>(STATE::WANDER),
		std::bind(&EnemyRat::ChangeStateWander, this));

	stateChanges_.emplace(static_cast<int>(STATE::MOVE_IN_RANGE),
		std::bind(&EnemyRat::ChangeStateMoveInRange, this));

	stateChanges_.emplace(static_cast<int>(STATE::SURPRISE),
		std::bind(&EnemyRat::ChangeStateSurprise, this));

	stateChanges_.emplace(static_cast<int>(STATE::CHASE),
		std::bind(&EnemyRat::ChangeStateChase, this));

	stateChanges_.emplace(static_cast<int>(STATE::ATTACK),
		std::bind(&EnemyRat::ChangeStateAttack, this));

	stateChanges_.emplace(static_cast<int>(STATE::ESCAPE),
		std::bind(&EnemyRat::ChangeStateEscape, this));

	stateChanges_.emplace(static_cast<int>(STATE::KNOCKBACK),
		std::bind(&EnemyRat::ChangeStateKnockBack, this));

	stateChanges_.emplace(static_cast<int>(STATE::DAMAGED),
		std::bind(&EnemyRat::ChangeStateDamaged, this));

	stateChanges_.emplace(static_cast<int>(STATE::DEAD),
		std::bind(&EnemyRat::ChangeStateDead, this));

	stateChanges_.emplace(static_cast<int>(STATE::END),
		std::bind(&EnemyRat::ChangeStateEnd, this));

	// 初期状態設定
	ChangeState(STATE::THINK);
}

void EnemyRat::UpdateProcess(void)
{
	const bool isInView =
		IsPlayerInViewRange(VIEW_RANGE, VIEW_ANGLE);
	if (!isInView)
	{
		canStartChase_ = true;
	}

	if ((state_ == STATE::IDLE
			|| state_ == STATE::WANDER
			|| state_ == STATE::MOVE_IN_RANGE)
		&& canStartChase_ && isInView)
	{
		canStartChase_ = false;
		ChangeState(STATE::SURPRISE);
		return;
	}

	// 状態別更新
	stateUpdate_();

	// ATTACK以外では攻撃判定を無効にする
	if (state_ != STATE::ATTACK)
	{
		SetAllAttackCollidersValid(false);
	}
}

void EnemyRat::UpdateProcessPost(void)
{
	if (uiSurprise_ != nullptr)
	{
		uiSurprise_->Update();
	}

	// 移動範囲外判定
	if (!InMovableRange()
		&& !(IsInValidDamage()
			|| state_ == STATE::MOVE_IN_RANGE
			|| state_ == STATE::SURPRISE
			|| state_ == STATE::CHASE
			|| state_ == STATE::ATTACK
			|| state_ == STATE::ESCAPE))
	{
		ChangeState(STATE::THINK);
	}
}

void EnemyRat::ChangeState(STATE state)
{
	if (state_ == STATE::SURPRISE && state != STATE::SURPRISE)
	{
		if (uiSurprise_ != nullptr)
		{
			uiSurprise_->SetActive(false);
		}

		if (state != STATE::CHASE)
		{
			canStartChase_ = true;
		}
	}

	state_ = state;

	// 各状態遷移の初期処理
	EnemyBase::ChangeState(static_cast<int>(state_));
}

void EnemyRat::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateNone, this);
}

void EnemyRat::ChangeStateThink(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateThink, this);

	if (!InMovableRange())
	{
		ChangeState(STATE::MOVE_IN_RANGE);
		return;
	}

	// 思考
	// ランダムに次の行動を決定
	// 30%で待機、70%で徘徊
	int rand = GetRand(100);
	if (rand < 30)
	{
		ChangeState(STATE::IDLE);
	}
	else
	{
		ChangeState(STATE::WANDER);
	}
}

void EnemyRat::ChangeStateIdle(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateIdle, this);

	// ランダムな待機時間
	step_ = 3.0f + static_cast<float>(GetRand(3));

	// 移動量ゼロ
	movePow_ = SchoolUtility::VECTOR_ZERO;

	// 待機アニメーション再生
	animController_->Play(
		static_cast<int>(ANIM_TYPE::IDLE), true);
}

void EnemyRat::ChangeStateWander(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateWander, this);

	// ランダムな角度
	float angle = static_cast<float>(GetRand(360)) * DX_PI_F / 180.0f;

	// 移動方向
	moveDir_ = VGet(sinf(angle), 0.0f, cosf(angle));

	// ランダムな移動時間
	step_ = 2.0f + static_cast<float>(GetRand(5));

	// 移動スピード
	moveSpeed_ = 3.0f;

	// 歩きアニメーション再生
	animController_->Play(
		static_cast<int>(ANIM_TYPE::WALK), true);
}

void EnemyRat::ChangeStateMoveInRange(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateMoveInRange, this);

	if (moveRadius_ <= 0.0f)
	{
		ChangeState(STATE::THINK);
		return;
	}

	VECTOR dir = VSub(transform_.pos, defaultPos_);
	dir.y = 0.0f;
	if (SchoolUtility::SqrMagnitude(dir) <= SchoolUtility::kEpsilonNormalSqrt)
	{
		ChangeState(STATE::THINK);
		return;
	}

	float safeRadius = moveRadius_ - MOVE_IN_RANGE_MARGIN;
	if (safeRadius <= MOVE_IN_RANGE_ARRIVE_RADIUS)
	{
		safeRadius = moveRadius_ * 0.8f;
	}

	dir = VNorm(dir);
	moveInRangeTargetPos_ = VAdd(defaultPos_, VScale(dir, safeRadius));
	moveInRangeTargetPos_.y = transform_.pos.y;

	moveSpeed_ = MOVE_IN_RANGE_SPEED;

	// 歩きアニメーション再生
	animController_->Play(
		static_cast<int>(ANIM_TYPE::WALK), true);
}

void EnemyRat::ChangeStateSurprise(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateSurprise, this);

	// 移動せずプレイヤーの方を向く
	movePow_ = SchoolUtility::VECTOR_ZERO;
	if (targetTransform_ != nullptr)
	{
		VECTOR targetDir =
			VSub(targetTransform_->pos, transform_.pos);
		targetDir.y = 0.0f;
		if (SchoolUtility::SqrMagnitude(targetDir)
			> SchoolUtility::kEpsilonNormalSqrt)
		{
			moveDir_ = VNorm(targetDir);
		}
	}

	// 待機アニメーション再生
	animController_->Play(
		static_cast<int>(ANIM_TYPE::IDLE), true);

	if (uiSurprise_ != nullptr)
	{
		uiSurprise_->Start();
	}
}

void EnemyRat::ChangeStateChase(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateChase, this);

	if (isResumeChase_)
	{
		isResumeChase_ = false;
	}
	else
	{
		step_ = CHASE_TIME;
	}

	moveSpeed_ = CHASE_SPEED;
	animController_->Play(
		static_cast<int>(ANIM_TYPE::WALK), true);
}

void EnemyRat::ChangeStateAttack(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateAttack, this);

	if (targetTransform_ == nullptr || attacks_.empty())
	{
		ChangeState(STATE::THINK);
		return;
	}

	attackIndex_ = 0;
	attackPhase_ = ATTACK_PHASE::PREPARE;
	movePow_ = SchoolUtility::VECTOR_ZERO;
	SetAllAttackCollidersValid(false);

	VECTOR targetDir = VSub(targetTransform_->pos, transform_.pos);
	targetDir.y = 0.0f;
	if (SchoolUtility::SqrMagnitude(targetDir)
		> SchoolUtility::kEpsilonNormalSqrt)
	{
		moveDir_ = VNorm(targetDir);
	}

	const ATTACK& attack = attacks_.at(attackIndex_);
	animController_->Play(attack.prepareAnimIndex, false, 0.1f);
	animController_->ResetPlayStep();
}

void EnemyRat::ChangeStateEscape(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateEscape, this);

	step_ = ESCAPE_TIME;
	moveSpeed_ = ESCAPE_SPEED;
	movePow_ = SchoolUtility::VECTOR_ZERO;
	SetAllAttackCollidersValid(false);

	// 歩きアニメーション再生
	animController_->Play(
		static_cast<int>(ANIM_TYPE::WALK), true);
}

void EnemyRat::ChangeStateKnockBack(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateKnockBack, this);

	// 被ダメアニメーション再生
	animController_->Play(
		static_cast<int>(ANIM_TYPE::HIT), false);
}

void EnemyRat::ChangeStateDamaged(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateDamaged, this);

	movePow_ = SchoolUtility::VECTOR_ZERO;

	animTypeDamaged_ = static_cast<int>(ANIM_TYPE::HIT);
	animController_->Play(animTypeDamaged_, false);
}

void EnemyRat::ChangeStateDead(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateDead, this);

	// 移動量ゼロ
	movePow_ = SchoolUtility::VECTOR_ZERO;

	// アニメーション終了後小さくする時間
	step_ = DEAD_END_STEP;

	// 撃破アニメーション再生
	animController_->Play(
		static_cast<int>(ANIM_TYPE::DIE), false);
}

void EnemyRat::ChangeStateEnd(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateEnd, this);

	// 完全に非表示にする
	Hide();
}

void EnemyRat::UpdateNone(void)
{
}

void EnemyRat::UpdateThink(void)
{

}

void EnemyRat::UpdateIdle(void)
{
	step_-= scnMng_.GetDeltaTime();
	if (step_ < 0.0f)
	{
		// 待機終了
		ChangeState(STATE::THINK);
		return;
	}
}

void EnemyRat::UpdateWander(void)
{
	if (ConsumeObstacleStuck())
	{
		// 障害物に詰まったら別の行動と移動方向を選び直す
		ChangeState(STATE::THINK);
		return;
	}

	step_ -= scnMng_.GetDeltaTime();
	if (step_ < 0.0f)
	{
		// 終了
		ChangeState(STATE::THINK);
		return;
	}

	ReserveObstacleStuckCheck(moveDir_);

	// 移動量(方向×スピード)
	movePow_ = VScale(moveDir_, moveSpeed_);
}

void EnemyRat::UpdateMoveInRange(void)
{
	VECTOR targetDir = VSub(moveInRangeTargetPos_, transform_.pos);
	targetDir.y = 0.0f;

	float arriveRadius = MOVE_IN_RANGE_ARRIVE_RADIUS * MOVE_IN_RANGE_ARRIVE_RADIUS;
	if (SchoolUtility::SqrMagnitude(targetDir) <= arriveRadius)
	{
		movePow_ = SchoolUtility::VECTOR_ZERO;
		ChangeState(STATE::THINK);
		return;
	}

	if (SchoolUtility::SqrMagnitude(targetDir) <= SchoolUtility::kEpsilonNormalSqrt)
	{
		movePow_ = SchoolUtility::VECTOR_ZERO;
		ChangeState(STATE::THINK);
		return;
	}

	moveDir_ = GetObstacleAvoidMoveDir(targetDir);
	movePow_ = VScale(moveDir_, moveSpeed_);
}

void EnemyRat::UpdateSurprise(void)
{
	movePow_ = SchoolUtility::VECTOR_ZERO;

	if (uiSurprise_ != nullptr && uiSurprise_->IsActive())
	{
		return;
	}

	if (IsPlayerInViewRange(VIEW_RANGE, VIEW_ANGLE))
	{
		canStartChase_ = false;
		ChangeState(STATE::CHASE);
	}
	else
	{
		ChangeState(STATE::THINK);
	}
}

void EnemyRat::UpdateChase(void)
{
	if (targetTransform_ == nullptr)
	{
		movePow_ = SchoolUtility::VECTOR_ZERO;
		ChangeState(STATE::THINK);
		return;
	}

	if (IsPlayerInAttackRange(ATTACK_RANGE))
	{
		movePow_ = SchoolUtility::VECTOR_ZERO;
		ChangeState(STATE::ATTACK);
		return;
	}

	step_ -= scnMng_.GetDeltaTime();

	// 視野内にまだプレイヤーがいるなら追跡時間延長
	if (IsPlayerInViewRange(VIEW_RANGE, VIEW_ANGLE)
		&& step_ < CHASE_CONTINUE)
	{
		step_ = CHASE_CONTINUE;
	}

	if (step_ <= 0.0f)
	{
		movePow_ = SchoolUtility::VECTOR_ZERO;
		ChangeState(STATE::THINK);
		return;
	}

	VECTOR targetDir =
		VSub(targetTransform_->pos, transform_.pos);
	targetDir.y = 0.0f;
	if (SchoolUtility::SqrMagnitude(targetDir)
		<= SchoolUtility::kEpsilonNormalSqrt)
	{
		movePow_ = SchoolUtility::VECTOR_ZERO;
		return;
	}

	moveDir_ = GetObstacleAvoidMoveDir(targetDir);
	movePow_ = VScale(moveDir_, moveSpeed_);
}

void EnemyRat::UpdateAttack(void)
{
	if (attacks_.empty())
	{
		SetAllAttackCollidersValid(false);
		ChangeState(STATE::THINK);
		return;
	}

	movePow_ = SchoolUtility::VECTOR_ZERO;

	const ATTACK& attack = attacks_.at(attackIndex_);

	if (attackPhase_ == ATTACK_PHASE::PREPARE)
	{
		SetAllAttackCollidersValid(false);

		if (animController_->IsEnd(attack.prepareAnimIndex))
		{
			attackPhase_ = ATTACK_PHASE::ATTACK;
			animController_->Play(attack.animIndex, false, 0.1f);
			animController_->ResetPlayStep();
		}
		return;
	}

	const AnimationController::Animation& anim =
		animController_->GetPlayAnim();

	float playRate = 0.0f;
	if (anim.totalTime > 0.0f)
	{
		playRate = anim.step / anim.totalTime;
	}
	SetAllAttackCollidersValid(
		attack.IsValidCollision(playRate));

	if (animController_->IsEnd(attack.animIndex))
	{
		SetAllAttackCollidersValid(false);
		if (step_ > 0.0f)
		{
			isResumeChase_ = true;
			ChangeState(STATE::CHASE);
		}
		else
		{
			ChangeState(STATE::THINK);
		}
	}
}

void EnemyRat::UpdateEscape(void)
{
	if (targetTransform_ == nullptr)
	{
		movePow_ = SchoolUtility::VECTOR_ZERO;
		ChangeState(STATE::THINK);
		return;
	}

	step_ -= scnMng_.GetDeltaTime();
	if (step_ <= 0.0f)
	{
		movePow_ = SchoolUtility::VECTOR_ZERO;
		ChangeState(STATE::THINK);
		return;
	}

	VECTOR escapeDir =
		VSub(transform_.pos, targetTransform_->pos);
	escapeDir.y = 0.0f;
	if (SchoolUtility::SqrMagnitude(escapeDir)
		<= SchoolUtility::kEpsilonNormalSqrt)
	{
		escapeDir = VScale(transform_.GetForward(), -1.0f);
		escapeDir.y = 0.0f;
	}

	moveDir_ = GetObstacleAvoidMoveDir(escapeDir);
	movePow_ = VScale(moveDir_, moveSpeed_);
}

void EnemyRat::UpdateDead(void)
{
	step_ -= scnMng_.GetDeltaTime();

	// アニメーション終了後、大きさを線形補間で小さくしていく
	if (animController_->IsEnd(static_cast<int>(ANIM_TYPE::DIE)))
	{

		// SchoolUtility::Lerpで大きさを小さくする
		transform_.scl = SchoolUtility::Lerp(
			transform_.scl,
			SchoolUtility::VECTOR_ZERO, 0.1f);
	}

	if (VSize(transform_.scl) <= SchoolUtility::kEpsilonNormalSqrt)
	{
		// 終了状態へ移行
		ChangeState(STATE::END);
	}
}

void EnemyRat::UpdateEnd(void)
{
}

void EnemyRat::DrawViewRangeProjection(void) const
{
	VECTOR projectionPoints
		[VIEW_PROJECTION_RADIUS_DIVISIONS + 1]
		[VIEW_PROJECTION_ANGLE_DIVISIONS + 1] = {};
	bool isProjectionValid
		[VIEW_PROJECTION_RADIUS_DIVISIONS + 1]
		[VIEW_PROJECTION_ANGLE_DIVISIONS + 1] = {};

	VECTOR centerPos = transform_.pos;
	VECTOR projectionCenter = {};
	const bool isCenterValid =
		GetViewProjectionPoint(centerPos, projectionCenter);
	for (int angleIndex = 0;
		angleIndex <= VIEW_PROJECTION_ANGLE_DIVISIONS;
		angleIndex++)
	{
		projectionPoints[0][angleIndex] = projectionCenter;
		isProjectionValid[0][angleIndex] = isCenterValid;
	}

	VECTOR forward = transform_.GetForward();
	forward.y = 0.0f;
	if (SchoolUtility::SqrMagnitude(forward)
		<= SchoolUtility::kEpsilonNormalSqrt)
	{
		return;
	}
	forward = VNorm(forward);

	const float forwardAngle = atan2f(forward.x, forward.z);
	for (int radiusIndex = 1;
		radiusIndex <= VIEW_PROJECTION_RADIUS_DIVISIONS;
		radiusIndex++)
	{
		const float radius = VIEW_RANGE
			* static_cast<float>(radiusIndex)
			/ static_cast<float>(VIEW_PROJECTION_RADIUS_DIVISIONS);

		for (int angleIndex = 0;
			angleIndex <= VIEW_PROJECTION_ANGLE_DIVISIONS;
			angleIndex++)
		{
			const float angleRate =
				static_cast<float>(angleIndex)
				/ static_cast<float>(VIEW_PROJECTION_ANGLE_DIVISIONS);
			const float localAngle =
				(-VIEW_ANGLE + VIEW_ANGLE * 2.0f * angleRate)
				* DX_PI_F / 180.0f;
			const float angle = forwardAngle + localAngle;

			VECTOR samplePos = centerPos;
			samplePos.x += sinf(angle) * radius;
			samplePos.z += cosf(angle) * radius;

			isProjectionValid[radiusIndex][angleIndex] =
				GetViewProjectionPoint(
					samplePos,
					projectionPoints[radiusIndex][angleIndex]);
		}
	}

	SetUseZBuffer3D(TRUE);
	SetWriteZBuffer3D(FALSE);
	SetDrawBlendMode(
		DX_BLENDMODE_ALPHA, VIEW_PROJECTION_ALPHA);

	unsigned int color = GetColor(0, 0, 255);
	if (state_ == STATE::SURPRISE)
	{
		color = GetColor(255, 255, 0);
	}
	else if (state_ == STATE::CHASE || state_ == STATE::ATTACK)
	{
		color = GetColor(255, 0, 0);
	}

	for (int angleIndex = 0;
		angleIndex < VIEW_PROJECTION_ANGLE_DIVISIONS;
		angleIndex++)
	{
		if (!isProjectionValid[0][angleIndex]
			|| !isProjectionValid[1][angleIndex]
			|| !isProjectionValid[1][angleIndex + 1])
		{
			continue;
		}

		DrawTriangle3D(
			projectionPoints[0][angleIndex],
			projectionPoints[1][angleIndex],
			projectionPoints[1][angleIndex + 1],
			color, TRUE);
	}

	for (int radiusIndex = 1;
		radiusIndex < VIEW_PROJECTION_RADIUS_DIVISIONS;
		radiusIndex++)
	{
		for (int angleIndex = 0;
			angleIndex < VIEW_PROJECTION_ANGLE_DIVISIONS;
			angleIndex++)
		{
			if (!isProjectionValid[radiusIndex][angleIndex]
				|| !isProjectionValid[radiusIndex][angleIndex + 1]
				|| !isProjectionValid[radiusIndex + 1][angleIndex]
				|| !isProjectionValid[radiusIndex + 1][angleIndex + 1])
			{
				continue;
			}

			DrawTriangle3D(
				projectionPoints[radiusIndex][angleIndex],
				projectionPoints[radiusIndex + 1][angleIndex],
				projectionPoints[radiusIndex + 1][angleIndex + 1],
				color, TRUE);
			DrawTriangle3D(
				projectionPoints[radiusIndex][angleIndex],
				projectionPoints[radiusIndex + 1][angleIndex + 1],
				projectionPoints[radiusIndex][angleIndex + 1],
				color, TRUE);
		}
	}

	SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	SetWriteZBuffer3D(TRUE);
}

bool EnemyRat::GetViewProjectionPoint(
	const VECTOR& pos, VECTOR& projectionPos) const
{
	VECTOR probeStart = pos;
	probeStart.y = transform_.pos.y + VIEW_PROJECTION_PROBE_UP;
	VECTOR probeEnd = pos;
	probeEnd.y = transform_.pos.y - VIEW_PROJECTION_PROBE_DOWN;

	bool isFoundGround = false;
	MV1_COLL_RESULT_POLY groundHit = {};
	for (const auto& hitCollider : hitColliders_)
	{
		if (hitCollider == nullptr || !hitCollider->IsValid()) continue;
		if (hitCollider->GetTag() != ColliderBase::TAG::STAGE) continue;
		if (hitCollider->GetShape() != ColliderBase::SHAPE::MODEL) continue;

		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(hitCollider);
		if (colliderModel == nullptr) continue;

		MV1_COLL_RESULT_POLY hit =
			colliderModel->GetNearestHitPolyLine(
				probeStart, probeEnd, true, false);
		if (!hit.HitFlag || hit.Normal.y <= 0.0f) continue;

		if (!isFoundGround
			|| hit.HitPosition.y > groundHit.HitPosition.y)
		{
			isFoundGround = true;
			groundHit = hit;
		}
	}

	if (!isFoundGround) return false;

	projectionPos = groundHit.HitPosition;
	projectionPos.y += VIEW_PROJECTION_Y_OFFSET;
	return true;
}

bool EnemyRat::IsPlayerInViewRange(
	float range, float viewHalfAngle) const
{
	if (targetTransform_ == nullptr || range < 0.0f
		|| viewHalfAngle < 0.0f) return false;

	VECTOR targetDir = VSub(targetTransform_->pos, transform_.pos);
	targetDir.y = 0.0f;
	const float targetDistanceSq = SchoolUtility::SqrMagnitude(targetDir);
	if (targetDistanceSq > range * range) return false;
	if (targetDistanceSq <= SchoolUtility::kEpsilonNormalSqrt) return true;

	VECTOR forward = transform_.GetForward();
	forward.y = 0.0f;
	if (SchoolUtility::SqrMagnitude(forward)
		<= SchoolUtility::kEpsilonNormalSqrt) return false;

	targetDir = VNorm(targetDir);
	forward = VNorm(forward);

	const float viewAngleRad =
		viewHalfAngle * DX_PI_F / 180.0f;
	const float minDot = cosf(viewAngleRad);
	return VDot(forward, targetDir) >= minDot;
}

bool EnemyRat::IsPlayerInAttackRange(float range) const
{
	if (targetTransform_ == nullptr || range < 0.0f) return false;

	VECTOR targetDir =
		VSub(targetTransform_->pos, transform_.pos);
	targetDir.y = 0.0f;
	return SchoolUtility::SqrMagnitude(targetDir)
		<= range * range;
}

bool EnemyRat::IsInValidDamage(void) const
{
	if (state_ == STATE::DEAD
		|| state_ == STATE::KNOCKBACK
		|| state_ == STATE::DAMAGED
		|| state_ == STATE::END)
	{
		return true;
	}
	return false;
}

void EnemyRat::OnStartKnockBack(void)
{
	// ノックバック状態へ移行
	ChangeState(STATE::KNOCKBACK);
}

void EnemyRat::OnEndKnockBack(void)
{
	if (hp_ == 0)
	{
		// 死亡状態へ移行
		ChangeState(STATE::DEAD);
	}
	else if (knockBackParam_.step <= DAMAGED_KNOCKBACK_TIME)
	{
		StartDamaged();
	}
	else
	{
		ChangeState(STATE::ESCAPE);
	}
}

void EnemyRat::OnStartDamaged(void)
{
	ChangeState(STATE::DAMAGED);
}

void EnemyRat::OnEndDamaged(void)
{
	if (hp_ == 0)
	{
		ChangeState(STATE::DEAD);
	}
	else
	{
		ChangeState(STATE::ESCAPE);
	}
}
