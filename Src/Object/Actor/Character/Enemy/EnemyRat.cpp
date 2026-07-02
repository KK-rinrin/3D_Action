#include "EnemyRat.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../Collider/ColliderLine.h"
#include "../../../Collider/ColliderCapsule.h"
#include "../../../Common/AnimationController.h"
#include "../../../../Utility/SchoolUtility.h"
#include "../../../../Manager/SceneManager.h"

EnemyRat::EnemyRat(const EnemyBase::EnemyData& data)
	:
	EnemyBase(data)
{
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
	animController_ = new AnimationController(transform_.modelId);
	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::IDLE), 30.0f, 8);

	animController_->AddInFbx(static_cast<int>(ANIM_TYPE::WALK), 30.0f, 13);

	animController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
}

void EnemyRat::InitPost(void)
{
	// 状態遷移初期処理登録
	stateChanges_.emplace(static_cast<int>(STATE::NONE),
		std::bind(&EnemyRat::ChangeStateNone, this));

	stateChanges_.emplace(static_cast<int>(STATE::THINK),
		std::bind(&EnemyRat::ChangeStateThink, this));

	stateChanges_.emplace(static_cast<int>(STATE::IDLE),
		std::bind(&EnemyRat::ChangeStateIdle, this));

	stateChanges_.emplace(static_cast<int>(STATE::WANDER),
		std::bind(&EnemyRat::ChangeStateWander, this));

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
	if (!InMovableRange())
	{
		// 移動前座標に戻す
		transform_.pos = prevPos_;
		transform_.Update();

		// 思考状態に戻す
		ChangeState(STATE::THINK);
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

void EnemyRat::ChangeStateEnd(void)
{
	stateUpdate_ = std::bind(&EnemyRat::UpdateEnd, this);
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

void EnemyRat::UpdateEnd(void)
{
}