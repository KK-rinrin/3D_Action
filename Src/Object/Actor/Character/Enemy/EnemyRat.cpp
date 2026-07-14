#include "EnemyRat.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../Collider/ColliderLine.h"
#include "../../../Collider/ColliderCapsule.h"
#include "../../../Common/AnimationController.h"
#include "../../../../Utility/SchoolUtility.h"
#include "../../../../Manager/SceneManager.h"

EnemyRat::EnemyRat(const EnemyBase::EnemyData& data)
	:
	EnemyBase(data),
	state_(STATE::NONE),
	step_(0.0f),
	moveInRangeTargetPos_(SchoolUtility::VECTOR_ZERO)
{
	knockBackParam_.weight = 50.0f;
}

EnemyRat::~EnemyRat(void)
{
}

void EnemyRat::InitLoad(void)
{
	// 基底クラスのリソースロード
	CharacterBase::InitLoad();

	// モデルの読み込み
	transform_.modelId = MV1DuplicateModel(resMng_.Load(ResourceManager::SRC::ENEMY_RAT).handleId_);
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

	stateChanges_.emplace(static_cast<int>(STATE::KNOCKBACK),
		std::bind(&EnemyRat::ChangeStateKnockBack, this));

	stateChanges_.emplace(static_cast<int>(STATE::DEAD),
		std::bind(&EnemyRat::ChangeStateDead, this));

	stateChanges_.emplace(static_cast<int>(STATE::END),
		std::bind(&EnemyRat::ChangeStateEnd, this));

	// 初期状態設定
	ChangeState(STATE::THINK);
}

void EnemyRat::UpdateProcess(void)
{
	// 状態別更新
	stateUpdate_();
}

void EnemyRat::UpdateProcessPost(void)
{
	// 移動範囲外判定
	if (!InMovableRange()
		&& !(IsInValidDamage() || state_ == STATE::MOVE_IN_RANGE))
	{
		ChangeState(STATE::MOVE_IN_RANGE);
	}
}

void EnemyRat::ChangeState(STATE state)
{
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

void EnemyRat::ChangeStateKnockBack(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateKnockBack, this);

	// 被ダメアニメーション再生
	animController_->Play(
		static_cast<int>(ANIM_TYPE::HIT), false);
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
	step_ -= scnMng_.GetDeltaTime();
	if (step_ < 0.0f)
	{
		// 終了
		ChangeState(STATE::THINK);
		return;
	}

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

	moveDir_ = VNorm(targetDir);
	movePow_ = VScale(moveDir_, moveSpeed_);
}

void EnemyRat::UpdateDead(void)
{
	step_ -= scnMng_.GetDeltaTime();

	// アニメーション終了後、大きさを線形補間で小さくしていく
	if (animController_->IsEnd())
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

bool EnemyRat::IsInValidDamage(void) const
{
	if (state_ == STATE::DEAD
		|| state_ == STATE::KNOCKBACK
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
	else if (!InMovableRange())
	{
		// 移動範囲外なら、移動範囲内へ戻る状態へ移行
		ChangeState(STATE::MOVE_IN_RANGE);
	}
	else
	{
		// それ以外は思考状態へ移行
		ChangeState(STATE::THINK);
	}
}