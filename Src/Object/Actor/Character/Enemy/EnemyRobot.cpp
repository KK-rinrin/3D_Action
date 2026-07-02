#include <cmath>
#include "../../../../Utility/SchoolUtility.h"
#include "../../../../Manager/ResourceManager.h"
#include "../../../../Manager/SceneManager.h"
#include "../../../Common/AnimationController.h"
#include "../../../Collider/ColliderLine.h"
#include "../../../Collider/ColliderCapsule.h"
#include "../../../Collider/ColliderCone.h"
#include "../../UI/UISurprise.h"
#include "../../../Shoot/ShotBase.h"
#include "EnemyRobot.h"

EnemyRobot::EnemyRobot(const EnemyBase::EnemyData& data)
	:
	EnemyBase(data),
	state_(STATE::NONE),
	step_(0.0f),
	wayPoints_(data.wayPoints),
	turningPhase_(0.0f),
	turningRight_(Quaternion::Identity()),
	turningLeft_(Quaternion::Identity()),
	activeWayPointIndex_(0),
	nextWayPoint_(SchoolUtility::VECTOR_ZERO),
	shootStep_(0.0f)
{
}

EnemyRobot::~EnemyRobot(void)
{
	shots_->Clear();
	delete shots_;
}

void EnemyRobot::Draw(void)
{
	// 基底クラスの描画処理
	CharacterBase::Draw();

	uiSurprise_->Draw();

	shots_->Draw();

#pragma region 視野(円錐)の描画
	SetUseLighting(FALSE);
	MV1DrawModel(viewRangeTransform_.modelId);
	SetUseLighting(TRUE);
#pragma endregion

#ifdef _DEBUG
	// 巡回ルート描画
	for (const auto& point : wayPoints_)
	{
		DrawSphere3D(
			point, 50.0f, 10,
			0x0000ff, 0x0000ff, false);
	}
#endif // _DEBUG
}

void EnemyRobot::InitLoad(void)
{
	// 基底クラスのリソースロード
	CharacterBase::InitLoad();

	// モデルのロード
	transform_.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::ENEMY_ROBOT));

	// 視野(円錐)モデルのロード
	viewRangeTransform_.SetModel(
		resMng_.LoadModelDuplicate(ResourceManager::SRC::VIEW_RANGE));

	// 「！」UIのロード
	uiSurprise_ = new UISurprise(&transform_, UI_LOCAL_HEIGHT, UI_BOUNCE_HEIGHT, UI_SIZE);
	uiSurprise_->Init();

	// 遠距離攻撃弾のインスタンスを生成
	shots_ = new ShotBase();
}

void EnemyRobot::InitTransform(void)
{
	// ロボット自身
	transform_.scl = VScale(SchoolUtility::VECTOR_ONE, SCALE);
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Euler(DEFAULT_LOCAL_ROT);
	transform_.Update();

	// 視野用円錐モデル
	viewRangeTransform_.scl = VIEW_RANGE_SCL;
	VECTOR pos = MV1GetFramePosition(transform_.modelId, VIEW_RANGE_SYNC_FRAME_IDX);
	viewRangeTransform_.pos = VAdd(pos, VIEW_POS_FIX);
	viewRangeTransform_.quaRot =
		transform_.quaRot.Mult(
			Quaternion::AngleAxis(VIEW_RANGE_ROT_X, SchoolUtility::AXIS_X));
	viewRangeTransform_.quaRotLocal =
		Quaternion::AngleAxis(VIEW_RANGE_LOCAL_ROT_X, SchoolUtility::AXIS_X);
	viewRangeTransform_.Update();

}

void EnemyRobot::InitCollider(void)
{
	// 主に地面との衝突で仕様する線分コライダ
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::ENEMY, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);
	ownColliders_.emplace(
		static_cast<int>(COLLIDER_TYPE::LINE), colLine);

	// 主に壁や木などの衝突で仕様するカプセルコライダ
	ColliderCapsule* colCapsule = new ColliderCapsule(
		ColliderBase::TAG::ENEMY, &transform_,
		COL_CAPSULE_TOP_LOCAL_POS,
		COL_CAPSULE_DOWN_LOCAL_POS, COL_CAPSULE_RADIUS);
	ownColliders_.emplace(
		static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);

	// 視野判定用円錐コライダ
	ColliderCone* colCone = new ColliderCone(
		ColliderBase::TAG::VIEW_RANGE, &viewRangeTransform_,
		COL_CONE_APEX_LOCAL_POS, COL_CONE_AXIS_LOCAL_DIR,
		VIEW_RANGE_PATROL,
		VIEW_ANGLE_PATROL * DX_PI_F / 180.0f,
		atanf(tanf(VIEW_ANGLE_PATROL * DX_PI_F / 180.0f) *
			VIEW_RANGE_VERTICAL_SCALE));
	ownColliders_.emplace(
		static_cast<int>(COLLIDER_TYPE::CONE), colCone);

}

void EnemyRobot::InitAnimation(void)
{
	animController_ = new AnimationController(transform_.modelId);
	// FBX内のアニメーション設定
	int type = -1;
	type = static_cast<int>(ANIM_TYPE::DANCE);
	animController_->AddInFbx(type, 10.0f, type);
	type = static_cast<int>(ANIM_TYPE::IDLE);
	animController_->AddInFbx(type, 20.0f, type);
	type = static_cast<int>(ANIM_TYPE::WALK);
	animController_->AddInFbx(type, 30.0f, type);
	type = static_cast<int>(ANIM_TYPE::RUN);
	animController_->AddInFbx(type, 30.0f, type);
	type = static_cast<int>(ANIM_TYPE::KICK);
	animController_->AddInFbx(type, 45.0f, type);
	type = static_cast<int>(ANIM_TYPE::SHOOT);
	animController_->AddInFbx(type, 30.0f, type);
	// 初期アニメーション再生
	animController_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);
}

void EnemyRobot::InitPost(void)
{
	// 状態遷移初期処理登録
	stateChanges_.emplace(static_cast<int>(STATE::NONE),
		std::bind(&EnemyRobot::ChangeStateNone, this));
	stateChanges_.emplace(static_cast<int>(STATE::THINK),
		std::bind(&EnemyRobot::ChangeStateThink, this));
	stateChanges_.emplace(static_cast<int>(STATE::IDLE),
		std::bind(&EnemyRobot::ChangeStateIdle, this));
	stateChanges_.emplace(static_cast<int>(STATE::PATROL),
		std::bind(&EnemyRobot::ChangeStatePatrol, this));
	stateChanges_.emplace(static_cast<int>(STATE::SURPRISE),
		std::bind(&EnemyRobot::ChangeStateSurprise, this));
	stateChanges_.emplace(static_cast<int>(STATE::ALERT),
		std::bind(&EnemyRobot::ChangeStateAlert, this));
	stateChanges_.emplace(static_cast<int>(STATE::CHASE),
		std::bind(&EnemyRobot::ChangeStateChase, this));
	stateChanges_.emplace(static_cast<int>(STATE::ATTACK_KICK),
		std::bind(&EnemyRobot::ChangeStateAttackKick, this));
	stateChanges_.emplace(static_cast<int>(STATE::ATTACK_SHOOT),
		std::bind(&EnemyRobot::ChangeStateAttackShoot, this));
	stateChanges_.emplace(static_cast<int>(STATE::ESCAPE),
		std::bind(&EnemyRobot::ChangeStateEscape, this));
	stateChanges_.emplace(static_cast<int>(STATE::DEAD),
		std::bind(&EnemyRobot::ChangeStateDead, this));
	stateChanges_.emplace(static_cast<int>(STATE::KNOCKBACK),
		std::bind(&EnemyRobot::ChangeStateKnockBack, this));
	stateChanges_.emplace(static_cast<int>(STATE::END),
		std::bind(&EnemyRobot::ChangeStateEnd, this));

	// 視野色を設定
	viewColors_[VIEW_COLOR::BLUE] = GetColorF(0.0f,0.0f,1.0f,0.5f);
	viewColors_[VIEW_COLOR::YELLOW] = GetColorF(1.0f, 1.0f, 0.0f, 0.5f);
	viewColors_[VIEW_COLOR::RED] = GetColorF(1.0f, 0.0f, 0.0f, 0.5f);

	// 初期状態設定
	ChangeState(STATE::THINK);

	// 視野色初期設定
	ChangeViewRangeColor(viewColors_[VIEW_COLOR::BLUE]);
}

void EnemyRobot::UpdateProcess(void)
{
	// 状態別更新
	stateUpdate_();

	// 弾の更新
	shots_->Update(scnMng_.GetDeltaTime());
}

void EnemyRobot::UpdateProcessPost(void)
{
	EnemyBase::UpdateProcessPost();

	// 警戒時「！」UI
	uiSurprise_->Update();

	// 視野用円錐モデル同期
	VECTOR pos = MV1GetFramePosition(transform_.modelId, VIEW_RANGE_SYNC_FRAME_IDX);
	viewRangeTransform_.pos = VAdd(pos, VIEW_POS_FIX);
	viewRangeTransform_.quaRot =
		transform_.quaRot.Mult(
			Quaternion::AngleAxis(VIEW_RANGE_ROT_X, SchoolUtility::AXIS_X));
	viewRangeTransform_.Update();
}

void EnemyRobot::ChangeState(STATE state)
{
	state_ = state;
	EnemyBase::ChangeState(static_cast<int>(state_));
}

void EnemyRobot::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateNone, this);
}

void EnemyRobot::ChangeStateThink(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateThink, this);

	// 思考
	// ランダムに次の行動を決定
	if (idleStep_ <= 0 && patrolStep_ <= 0)
	{
		idleStep_ = 1 + GetRand(1);	// 1～2

		patrolStep_ = 1 + GetRand(2);	// 1～3
	}
}

void EnemyRobot::ChangeStateIdle(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateIdle, this);

	// ランダムな待機時間
	step_ = IDLE_TIME + static_cast<float>(GetRand(IDLE_TIME_RAND));

	// 移動量ゼロ
	movePow_ = SchoolUtility::VECTOR_ZERO;

	// 待機アニメーション再生
	animController_->Play(
		static_cast<int>(ANIM_TYPE::IDLE), true);

	// 視野色変更
	ChangeViewRangeColor(viewColors_[VIEW_COLOR::BLUE]);
}

void EnemyRobot::ChangeStatePatrol(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdatePatrol, this);

	// 移動量ゼロ
	movePow_ = SchoolUtility::VECTOR_ZERO;

	if (activeWayPointIndex_ == wayPoints_.size())
	{
		// 巡回終了
		activeWayPointIndex_ = 0;
		ChangeState(STATE::THINK);
		return;
	}

	// 次の巡回ポイント更新
	nextWayPoint_ = wayPoints_[activeWayPointIndex_];

	// 巡回ルートの移動方向を設定する
	SetMoveDirPatrol();

	// 移動スピード
	moveSpeed_ = 5.0f;

	// 歩きアニメーション再生
	animController_->Play(
		static_cast<int>(ANIM_TYPE::WALK), true);

	// 視野色変更
	ChangeViewRangeColor(viewColors_[VIEW_COLOR::BLUE]);
}

void EnemyRobot::ChangeStateSurprise(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateSurprise, this);

	// 移動量ゼロ
	movePow_ = SchoolUtility::VECTOR_ZERO;

	// 待機アニメーション再生
	animController_->Play(static_cast<int>(ANIM_TYPE::IDLE), true);

	// 今の向きのまま
	moveDir_ = transform_.GetForward();

	// UI再生
	uiSurprise_->Start();

	// 立ち止まる時間
	step_ = SURPRISE_TIME;

	// 視野色変更
	ChangeViewRangeColor(viewColors_[VIEW_COLOR::YELLOW]);
}

void EnemyRobot::ChangeStateAlert(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateAlert, this);

	// 移動量ゼロ
	movePow_ = SchoolUtility::VECTOR_ZERO;

	// 現在の向きを基準に、振り向きの右端・左端を生成
	const float turningRad = SchoolUtility::Deg2RadF(TURNING_ANGLE);

	turningRight_ = transform_.quaRot.Mult(
		Quaternion::AngleAxis(turningRad, SchoolUtility::AXIS_Y));

	turningLeft_ = transform_.quaRot.Mult(
		Quaternion::AngleAxis(-turningRad, SchoolUtility::AXIS_Y));

	// 初期化
	turningPhase_ = 0.0f;

	// ダンス(足踏み)アニメーション再生
	animController_->Play(
		static_cast<int>(ANIM_TYPE::DANCE), true);

	// 視野色変更
	ChangeViewRangeColor(viewColors_[VIEW_COLOR::YELLOW]);
}

void EnemyRobot::ChangeStateChase(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateChase, this);

	// 移動速度
	moveSpeed_ = CHASE_SPEED;

	if (step_ <= 0.0f)
	{
		// 追跡する時間を設定
		step_ = CHASE_TIME;
	}

	// 遠距離攻撃開始までの時間
	shootStep_ = 2.0f + static_cast<float>(GetRand(RAND_TIME)) / 100.0f;
	
	// 走るアニメーション再生
	animController_->Play(static_cast<int>(ANIM_TYPE::RUN), true);

	// 視野色変更
	ChangeViewRangeColor(viewColors_[VIEW_COLOR::RED]);
}

void EnemyRobot::ChangeStateAttackKick(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateAttackKick, this);

	if (targetTransform_ == nullptr)
	{
		// ふぉーるばっく
		ChangeState(STATE::THINK);
	}

	// プレイヤーを向く
	moveDir_ = GetLookPlayerXZ();

	// クールタイムを設定
	step_ = ATTACK_COOLTIME;

	// キックアニメーション再生
	animController_->Play(static_cast<int>(ANIM_TYPE::KICK), false);
}

void EnemyRobot::ChangeStateAttackShoot(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateAttackShoot, this);

	// プレイヤーを向く
	moveDir_ = GetLookPlayerXZ();

	// 移動量ゼロ
	movePow_ = SchoolUtility::VECTOR_ZERO;

	// クールタイム
	shootStep_ = ATTACK_COOLTIME;

	shots_->SpawnShot(MV1GetFramePosition(transform_.modelId, SHOT_SYNC_FRAME_IDX), 
		VScale(VNorm(moveDir_),SHOT_SPEED), SHOT_RADIUS);

	// アニメーション再生
	animController_->Play(static_cast<int>(ANIM_TYPE::SHOOT), false);
}

void EnemyRobot::ChangeStateEscape(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateEscape, this);
}

void EnemyRobot::ChangeStateDead(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateDead, this);
}

void EnemyRobot::ChangeStateKnockBack(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateKnockBack, this);
}

void EnemyRobot::ChangeStateEnd(void)
{
	stateUpdate_ = std::bind(&EnemyRobot::UpdateEnd, this);
}

void EnemyRobot::UpdateNone(void)
{
}

void EnemyRobot::UpdateThink(void)
{
	if (idleStep_ > 0)
	{
		idleStep_--;
		ChangeState(STATE::IDLE);
	}
	else if (patrolStep_ > 0)
	{
		patrolStep_--;
		ChangeState(STATE::PATROL);
	}
}

void EnemyRobot::UpdateIdle(void)
{
	if (DetectPlayer())
	{
		ChangeState(STATE::SURPRISE);
		return;
	}

	step_ -= scnMng_.GetDeltaTime();
	if (step_ < 0.0f)
	{
		// 待機終了
		ChangeState(STATE::THINK);
		return;
	}
}

void EnemyRobot::UpdatePatrol(void)
{
	if (DetectPlayer())
	{
		ChangeState(STATE::SURPRISE);
		return;
	}

	// 巡回ポイントとの球体衝突判定(半径30.0fくらい)
	// if (VSize(VSub(transform_.pos, nextWayPoint_)) < 30.0f)
	if (SchoolUtility::IsHitSphere(nextWayPoint_, 30.0f, transform_.pos))
	{
		// 巡回ポイントインデックス更新
		activeWayPointIndex_++;

		// 次の移動地点へか、待機か思考
		ChangeState(STATE::THINK);
		return;
	}
	// 巡回ルートの移動方向を設定する
	SetMoveDirPatrol();

	// 移動量の計算
	movePow_ = VScale(moveDir_, moveSpeed_);
}

void EnemyRobot::UpdateSurprise(void)
{
	// 待機する
	step_ -= scnMng_.GetDeltaTime();
	if (step_ < 0.0f)
	{
		// 警戒へ
		ChangeState(STATE::ALERT);
		return;
	}
}

void EnemyRobot::UpdateAlert(void)
{
	// 検出したら追跡する
	if (DetectPlayer())
	{
		ChangeState(STATE::CHASE);
		return;
	}

	// 中央から左右へ滑らかに往復する
	turningPhase_ += scnMng_.GetDeltaTime() * TURNING_SPEED;

	// 0～最大値に収める（多分必要ない）
	turningPhase_ = SchoolUtility::Clamp(
		turningPhase_, 0.0f,
		static_cast<float>(TURNING_ROUND_TRIP_MAX));

	// 補間レートを0.5から始めて0.0～1.0 に丸め込む
	const float turningRate =
		0.5f - sinf(turningPhase_ * DX_TWO_PI_F) * 0.5f;

	// 球面補間
	transform_.quaRot = Quaternion::Slerp(
		turningRight_, turningLeft_, turningRate);

	// 現在の向きから移動方向を更新
	moveDir_ = transform_.quaRot.GetForward();

	// そのまま2往復したら待機へ
	if (turningPhase_ >= static_cast<float>(TURNING_ROUND_TRIP_MAX))
	{
		ChangeState(STATE::IDLE);
		return;
	}
}

void EnemyRobot::UpdateChase(void)
{
	step_ -= scnMng_.GetDeltaTime();

	// 追跡相手のTransformが設定されていないか、
	// ステップがなくなったらTHINKへ
	if (targetTransform_ == nullptr || step_ < 0.0f)
	{
		step_ = 0.0f;
		ChangeState(STATE::THINK);
		return;
	}

	shootStep_ -= scnMng_.GetDeltaTime();
	// 一定時間経過したら遠距離攻撃に入る
	if (shootStep_ < 0.0f)
	{
		ChangeState(STATE::ATTACK_SHOOT);
		return;
	}

	// プレイヤーを攻撃できる範囲に入ったら近接攻撃に入る
	if (GetDistanceToP() <= ATTACK_RANGE)
	{
		step_ = 0.0f;
		ChangeState(STATE::ATTACK_KICK);
		return;
	}

	// 視野内にまだプレイヤーがいるなら追跡時間延長
	if (DetectPlayer() && step_ < CHASE_CONTINUE)
	{
		step_ = CHASE_CONTINUE;
	}

	// プレイヤーへの方向を取得
	VECTOR targetDir = GetLookPlayerXZ();

	if (SchoolUtility::SqrMagnitude(targetDir) <= SchoolUtility::kEpsilonNormalSqrt)
	{
		movePow_ = SchoolUtility::VECTOR_ZERO;
		return;
	}
	
	// プレイヤーへの方向を適用
	moveDir_ = VNorm(targetDir);

	// 移動量
	movePow_ = VScale(moveDir_, moveSpeed_);
}

void EnemyRobot::UpdateAttackKick(void)
{
	step_ -= scnMng_.GetDeltaTime();

	// 一度攻撃して、クールタイム消費後に…
	if (step_ < 0.0f)
	{
		// まだ攻撃範囲にいたら繰り返す
		if (GetDistanceToP() <= ATTACK_RANGE) ChangeState(STATE::ATTACK_KICK);

		// 離れているなら再び追跡を開始する
		else ChangeState(STATE::CHASE);
		
		return;
	}

	// 距離が離れてるなら近づく
	VECTOR targetMovePow = SchoolUtility::VECTOR_ZERO;
	if (GetDistanceToP() > TOO_NEAR_RANGE && !animController_->IsEnd())
	{
		VECTOR targetDir = GetLookPlayerXZ();
		if (SchoolUtility::SqrMagnitude(targetDir) > SchoolUtility::kEpsilonNormalSqrt)
		{
			moveDir_ = VNorm(targetDir);
			targetMovePow = VScale(moveDir_, KICK_APPROACH_SPEED);
		}
	}

	movePow_ = SchoolUtility::Lerp(
		movePow_, targetMovePow, KICK_APPROACH_LERP_RATE);

	// キックモーションが終わったら
	if (animController_->IsEnd())
	{
		// 移動量をゼロにする
		movePow_ = SchoolUtility::VECTOR_ZERO;
	}
}
void EnemyRobot::UpdateAttackShoot(void)
{
	shootStep_ -= scnMng_.GetDeltaTime();

	if (shootStep_ < 0.0f)
	{
		ChangeState(STATE::CHASE);
		return;
	}

	
}

void EnemyRobot::UpdateEscape(void)
{
}

void EnemyRobot::UpdateDead(void)
{
}

void EnemyRobot::UpdateKnockBack(void)
{
}

void EnemyRobot::UpdateEnd(void)
{
}

bool EnemyRobot::DetectPlayer(void)
{
	int coneType = static_cast<int>(COLLIDER_TYPE::CONE);
	if (ownColliders_.count(coneType) == 0) return false;

	const ColliderCone* colliderCone =
		dynamic_cast<const ColliderCone*>(ownColliders_.at(coneType));
	if (colliderCone == nullptr) return false;

	for (const auto& hitCol : hitColliders_)
	{
		if (hitCol == nullptr) continue;
		if (hitCol->GetTag() != ColliderBase::TAG::PLAYER) continue;
		if (hitCol->GetShape() != ColliderBase::SHAPE::CAPSULE) continue;

		const ColliderCapsule* colliderCapsule =
			dynamic_cast<const ColliderCapsule*>(hitCol);
		if (colliderCapsule == nullptr) continue;

		if (colliderCone->IsHitCapsule(colliderCapsule))
		{
			return true;
		}
	}

	return false;
}

void EnemyRobot::SetMoveDirPatrol(void)
{
	// 巡回先座標XZ
	VECTOR tmpPos = nextWayPoint_;
	tmpPos.y = 0.0f;

	// 現在地座標XZ
	VECTOR pos = transform_.pos;
	pos.y = 0.0f;

	// XZ平面上の移動方向を計算
	moveDir_ = VNorm(VSub(tmpPos, pos));
}

VECTOR EnemyRobot::GetLookPlayerXZ(void)
{
	if (targetTransform_ == nullptr)
	{
		return SchoolUtility::VECTOR_ZERO;
	}

	// 自分自身の座標を取得
	VECTOR ePos = transform_.pos;

	// プレイヤー座標取得
	VECTOR pPos = targetTransform_->pos;

	// XZ平面
	pPos.y = ePos.y = 0.0f;

	// プレイヤーの方向を返す（大きさは１）
	return VNorm(VSub(pPos, ePos));
}

void EnemyRobot::ChangeViewRangeColor(COLOR_F& color) const
{
	// 拡散光を変更
	MV1SetMaterialDifColor(
		viewRangeTransform_.modelId, 0, color);
}

float EnemyRobot::GetDistanceToP()
{
	if (targetTransform_ == nullptr) return -1.0f;
	return static_cast<float>(SchoolUtility::Distance(targetTransform_->pos,transform_.pos));
}