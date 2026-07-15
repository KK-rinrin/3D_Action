#include "../../../Utility/SchoolUtility.h"
#include "../../../Application.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/SceneManager.h"
#include "../../../Manager/InputBinder.h"
#include "../../../Manager/Camera.h"
#include "../../Common/AnimationController.h"
#include "../../Collider/ColliderLine.h"
#include "../../Collider/ColliderCapsule.h"
#include "../Weapon/WeaponBlade.h"
#include "Player.h"

VECTOR Player::GetMoveInputDir(void)
{
	VECTOR inputDir = inputBinder_->GetDirection();

	if (SchoolUtility::EqualsVZero(inputDir))
	{
		return SchoolUtility::VECTOR_ZERO;
	}

	// 移動方向をカメラの回転に合わせる
	VECTOR camForward = SchoolUtility::DIR_F;
	VECTOR camRight = SchoolUtility::DIR_R;
	auto camera = SceneManager::GetInstance().GetCamera();
	if (camera != nullptr)
	{
		camForward = camera->GetForward();
		camRight = camera->GetQuaRotY().GetRight();
	}

	camForward.y = 0.0f;
	camRight.y = 0.0f;
	camForward = SchoolUtility::VNormalize(camForward);
	camRight = SchoolUtility::VNormalize(camRight);

	VECTOR dirX = VScale(camRight, inputDir.x);
	VECTOR dirZ = VScale(camForward, inputDir.z);
	return SchoolUtility::VNormalize(VAdd(dirX, dirZ));
}

Player::Player(InputBinder* inputBinder)
	:CharacterBase(),
	weapon_(nullptr),
	state_(STATE::NONE),
	inputBinder_(inputBinder)
{
	
}

Player::~Player()
{
	delete weapon_;
}

//void Player::Update()
//{
//	// 移動操作
//	ProcessMove();
// 
//	// 移動処理
//	transform_.pos = VAdd(transform_.pos, movePow_);
// 
//	// 3Dの基礎情報更新
//	transform_.Update();
// 
//	// アニメーション更新
//	animController_->Update();
//
//}

void Player::Draw()
{
	CharacterBase::Draw();

	weapon_->Draw();
}

void Player::Release()
{
}

WeaponBase* Player::GetWeapon(void)
{
	return weapon_;
}

void Player::InitLoad(void)
{
	CharacterBase::InitLoad();

	transform_.SetModel(resMng_.Load(ResourceManager::SRC::PLAYER).handleId_);

	// 武器をロード（右手の位置はフレーム番号44）
	weapon_ = new WeaponBlade(transform_,44);
	weapon_->Init();

}

void Player::InitTransform(void)
{
	transform_.scl = VScale(SchoolUtility::VECTOR_ONE, SCALE);
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Euler(INIT_ROT);
	transform_.pos = INIT_POS;
}

void Player::InitCollider(void)
{
	// 主に地面との衝突で仕様する線分コライダ
	ColliderLine* colLine = new ColliderLine(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_LINE_START_LOCAL_POS, COL_LINE_END_LOCAL_POS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::LINE), colLine);

	// 主に壁や木などの衝突で仕様するカプセルコライダ
	ColliderCapsule* colCapsule = new ColliderCapsule(
		ColliderBase::TAG::PLAYER, &transform_,
		COL_CAPSULE_TOP_LOCAL_POS, COL_CAPSULE_DOWN_LOCAL_POS,
		COL_CAPSULE_RADIUS);
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::CAPSULE), colCapsule);
}

void Player::InitAnimation(void)
{
	std::string path = Application::PATH_MODEL + "Player/";
	animController_ = new AnimationController(transform_.modelId);
	animController_->Add(static_cast<int>(ANIM_TYPE::IDLE), 30.0f, path + "Idle.mv1");
	animController_->Add(static_cast<int>(ANIM_TYPE::RUN), 30.0f, path + "Run.mv1");
	animController_->Add(static_cast<int>(ANIM_TYPE::FAST_RUN), 30.0f, path + "FastRun.mv1");
	animController_->Add(
		static_cast<int>(ANIM_TYPE::JUMP), 60.0f, path + "JumpRising.mv1");
	animController_->Add(
		static_cast<int>(
			ANIM_TYPE::SPINNING_SLASH), 60.0f, path + "SpinningSlash.mv1");
	animController_->Add(
		static_cast<int>(
			ANIM_TYPE::VERTICAL_SLASH), 60.0f, path + "VerticalSlash.mv1");	
	animController_->Add(
		static_cast<int>(
			ANIM_TYPE::HORIZONTAL_SLASH), 60.0f, path + "HorizontalSlash.mv1");

	animController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
}

void Player::InitPost(void)
{
	// 攻撃コンボデータ
	ATTACK_COMBO data = ATTACK_COMBO();

	// 横切り攻撃(0.0～72.0)
	// コンボ受付開始、衝突判定開始
	data = { 15.0f, 50.0f, 24.0f, 38.0f, 55.0f, 8.0f, false, 0.0f };
	atkComboData_.emplace(
		STATE_ATTACK_COMBO::HORIZONTAL, data);

	// 縦切り攻撃(0.0～68.0)
	// コンボ受付開始、衝突判定開始
	data = { 10.0f, 40.0f, 18.0f, 32.0f, 50.0f, 5.0f, false, 0.0f };
	atkComboData_.emplace(
		STATE_ATTACK_COMBO::VERTICAL, data);

	// 回転攻撃
	// コンボ受付開始、衝突判定開始
	data = { 0.0f, 0.0f, 26.0f, 34.0f, 95.0f, 12.0f, false, 1.0f };
	atkComboData_.emplace(
		STATE_ATTACK_COMBO::SPINNING, data);

	// 状態管理
	stateChanges_.emplace(
		STATE::NONE, std::bind(&Player::ChangeStateNone, this));
	stateChanges_.emplace(
		STATE::PLAY, std::bind(&Player::ChangeStatePlay, this));
	stateChanges_.emplace(
		STATE::ATTACK, std::bind(&Player::ChangeStateAttack, this));
	// 初期状態
	ChangeState(STATE::PLAY);

	weapon_->Update();
}

void Player::UpdateProcess(void)
{
	// 現在のステートの更新
	stateUpdate_();
}
void Player::UpdateProcessPost(void)
{
	weapon_->Update();
}

void Player::ProcessMove(void)
{
	bool isDash = inputBinder_->IsDash();
	moveDir_ = GetMoveInputDir();

	isDash_ = isDash && !SchoolUtility::EqualsVZero(moveDir_);

	if (!SchoolUtility::EqualsVZero(moveDir_))
	{
		moveSpeed_ = isDash ? SPEED_DASH : SPEED_MOVE;

		if (!isJump_)
		{
			animController_->Play(static_cast<int>(isDash ? ANIM_TYPE::FAST_RUN : ANIM_TYPE::RUN));
		}
	}
	else
	{
		moveSpeed_ = 0.0f;
		if (!isJump_) animController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
		moveDir_ = SchoolUtility::VECTOR_ZERO;
	}

	movePow_ = VScale(moveDir_, moveSpeed_);
}

void Player::ProcessJump(void)
{
	// 持続ジャンプ処理
	if (inputBinder_->IsJump())
	{
		// ジャンプの入力受付時間を減少
		stepJump_ += scnMng_.GetDeltaTime();
		if (stepJump_ <= TIME_JUMP_INPUT)
		{
			// ジャンプ量の計算
			float jumpSpeed = POW_JUMP_KEEP * scnMng_.GetDeltaTime();
			jumpPow_ = VAdd(jumpPow_, VScale(SchoolUtility::DIR_U, jumpSpeed));
		}
	}
	else
	{
		// ボタンを離したらジャンプ力に加算しない
		stepJump_ = TIME_JUMP_INPUT;
	}

	// 初期ジャンプ処理
	if (inputBinder_->IsTrgJump() && !isJump_)
	{
		// ジャンプ量の計算
		float jumpSpeed = POW_JUMP_INIT * scnMng_.GetDeltaTime();
		jumpPow_ = VScale(SchoolUtility::DIR_U, jumpSpeed);
		isJump_ = true;
		// アニメーション再生
		animController_->Play(
			static_cast<int>(ANIM_TYPE::JUMP), false);
	}
}

void Player::ProcessAttack(void)
{
	// ジャンプ中は攻撃できない
	if (isJump_) return;

	// 攻撃ボタン
	bool isHitKey = inputBinder_->IsAttack();

	if (isHitKey)	// 旧コードでは条件にこれを追加→ || isTrgDownWhileAttack_
	{
		// 連続攻撃回数を加算（旧式）
		// attackChain_++;

		// 最初の攻撃は横斬りから始める
		stateAtkCombo_ = STATE_ATTACK_COMBO::HORIZONTAL;
		ChangeState(STATE::ATTACK);
		return;
	}

	// 連続攻撃記録をリセット（旧式）
	// attackChain_ = 0;
}

void Player::ChangeState(STATE state)
{
	// 状態変更
	state_ = state;
	// 各状態遷移の初期処理
	stateChanges_[state_]();
}

void Player::ChangeStateNone(void)
{
	stateUpdate_ = std::bind(&Player::UpdateNone, this);
}

void Player::ChangeStatePlay(void)
{
	stateUpdate_ = std::bind(&Player::UpdatePlay, this);
}

void Player::ChangeStateAttack(void)
{
	stateUpdate_ = std::bind(&Player::UpdateAttack, this);

	const ATTACK_COMBO& data = atkComboData_.at(stateAtkCombo_);	// コンボデータを取得
	weapon_->SetAllColliderKnockBackPow(data.knockBackPow);
	VECTOR inputDir = GetMoveInputDir();	// 移動
	if (!SchoolUtility::EqualsVZero(inputDir))
	{
		VECTOR blendedDir = SchoolUtility::Lerp(
			moveDir_, inputDir, ATTACK_DIR_LERP_RATE);
		moveDir_ = SchoolUtility::VNormalize(blendedDir);
		moveSpeed_ = data.moveSpeed + (isDash_ ? SPEED_DASH * ATTACK_DASH_MOVE_DECAY : 0.0f);
		movePow_ = VScale(moveDir_, moveSpeed_);
	}
	else
	{
		moveDir_ = SchoolUtility::VECTOR_ZERO;
		moveSpeed_ = 0.0f;
		movePow_ = SchoolUtility::VECTOR_ZERO;
	}

	switch (stateAtkCombo_)
	{
	case Player::STATE_ATTACK_COMBO::HORIZONTAL:
		// 横切りアニメーション再生
		animController_->Play(
			static_cast<int>(ANIM_TYPE::HORIZONTAL_SLASH), false);
		break;
	case Player::STATE_ATTACK_COMBO::VERTICAL:
		// 縦切りアニメーション再生
		animController_->Play(
			static_cast<int>(ANIM_TYPE::VERTICAL_SLASH), false);
		break;
	case Player::STATE_ATTACK_COMBO::SPINNING:
		// スピニングスラッシュアニメーション再生
		animController_->Play(
			static_cast<int>(ANIM_TYPE::SPINNING_SLASH), false);
		break;
	default:
		break;
	}

#pragma region ～旧コード～
#if 0
	// 連続攻撃判定スタックを消す
	isTrgDownWhileAttack_ = false;

	// スラッシュアニメーション再生
	if (attackChain_ % ATTACK_VARIOUS == comboIndex[ANIM_TYPE::VERTICAL_SLASH])
	{
		animController_->Play(
			static_cast<int>(ANIM_TYPE::VERTICAL_SLASH), false);
	}
	else if (attackChain_ % ATTACK_VARIOUS == comboIndex[ANIM_TYPE::HORIZONTAL_SLASH])
	{
	animController_->Play(
		static_cast<int>(ANIM_TYPE::HORIZONTAL_SLASH), false);
	}
	else if (attackChain_ % ATTACK_VARIOUS == comboIndex[ANIM_TYPE::SPINNING_SLASH])
	{
		animController_->Play(
			static_cast<int>(ANIM_TYPE::SPINNING_SLASH), false);
	}
#endif
#pragma endregion
}

void Player::UpdateNone(void)
{
}

void Player::UpdatePlay(void)
{
	// 移動操作
	ProcessMove();

	// ジャンプ処理
	ProcessJump();

	// 攻撃処理
	ProcessAttack();
}

void Player::UpdateAttack(void)
{
	// 移動量を徐々に減少
	movePow_ = SchoolUtility::Lerp(movePow_, SchoolUtility::VECTOR_ZERO, 0.05f);

	// コンボデータを取得
	ATTACK_COMBO& data = atkComboData_.at(stateAtkCombo_);

	// アニメーションの再生Stepを取得
	float step = animController_->GetPlayAnim().step;

	// 武器の衝突判定設定
	weapon_->SetAllColliderValid(data.IsValidCollsion(step));

	// コンボ判定
	if (!data.isNextCombo)
	{
		bool isValidCombo = data.IsValidCombo(step);
		if (isValidCombo)
		{
			// 攻撃ボタン
			data.isNextCombo = inputBinder_->IsAttack();
		}
	}
	// コンボ可能
	if (data.isNextCombo)
	{
		// アニメーション終了か割り込み有効か
		if (animController_->IsEnd() || data.IsValidInterrupt(step))
		{
			// 残りコンボ判定
			int state = static_cast<int>(stateAtkCombo_);
			state++;
			int max = static_cast<int>(STATE_ATTACK_COMBO::MAX);
			if (state < max)
			{
				// 次のコンボへ
				stateAtkCombo_ = static_cast<STATE_ATTACK_COMBO>(state);
				
				ChangeStateAttack();
				return;
			}
		}
	}

	// アニメーションの再生終了
	if (animController_->IsEnd())
	{
		// コンボリセット
		for (auto& data : atkComboData_)
		{
			data.second.isNextCombo = false;
		}

		ChangeState(STATE::PLAY);
	}

#pragma region ～旧コード～
	// InputManagerで動作するため、このまま有効にしたらコンパイルエラーになります
#if 0
	// 攻撃アニメーションの再生ステップを取得
	auto animStep = animController_->GetPlayAnim().step;

	auto& ins = InputManager::GetInstance();
	// 攻撃が終わる直前にボタンを押したら次の攻撃を準備
	if ((ins.IsTrgDown(KEY_INPUT_COLON) || ins.IsTrgDown(KEY_INPUT_X)) &&
		animStep > animController_->GetPlayAnim().totalTime - BUTTON_WHILE_ATTACK_STEP)
		isTrgDownWhileAttack_ = true;

	// 攻撃アニメーションの種類を取得
	int atkVar = attackChain_ % ATTACK_VARIOUS;
	float start = 0.0f;
	float end = 0.0f;

	// 攻撃アニメーションの有効フレームを取得
	if (isAttackAnim(animController_->GetPlayType())) {
		auto frame = get(toAttackAnim(animController_->GetPlayType()));
		start = frame.start;
		end = frame.end;
	}
	
	bool isValid = false;
	// 攻撃アニメーションの有効フレーム内で武器のコライダを有効化
	if (animStep >= start && animStep <= end)
		isValid = true;
	weapon_->SetAllColliderValid(isValid);

	// アニメーションの再生終了
	if (animController_->IsEnd())
	{
		ChangeState(STATE::PLAY);
	}
#endif
#pragma endregion
}

void Player::CollisionReserve(void)
{
	// アニメーションごとの線分・カプセル調整
	if (animController_->GetPlayType() == static_cast<int>(ANIM_TYPE::JUMP))
	{
		// ジャンプ中は線分を伸ばす
		if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::LINE)) != 0)
		{
			ColliderLine* colLine = dynamic_cast<ColliderLine*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::LINE)));
			colLine->SetLocalPosStart(COL_LINE_JUMP_START_LOCAL_POS);
			colLine->SetLocalPosEnd(COL_LINE_JUMP_END_LOCAL_POS);
		}

		// ジャンプ中はカプセルを移動する
		if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::CAPSULE)) != 0)
		{
			ColliderCapsule* colCap = dynamic_cast<ColliderCapsule*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::CAPSULE)));
			colCap->SetLocalPosTop(COL_CAPSULE_TOP_JUMP_LOCAL_POS);
			colCap->SetLocalPosDown(COL_CAPSULE_DOWN_JUMP_LOCAL_POS);
		}
	}
	else
	{
		// 通常時の線分に戻す
		if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::LINE)) != 0)
		{
			ColliderLine* colLine = dynamic_cast<ColliderLine*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::LINE)));
			colLine->SetLocalPosStart(COL_LINE_START_LOCAL_POS);
			colLine->SetLocalPosEnd(COL_LINE_END_LOCAL_POS);
		}

		// 通常時のカプセルに戻す
		if (ownColliders_.count(static_cast<int>(COLLIDER_TYPE::CAPSULE)) != 0)
		{
			ColliderCapsule* colCap = dynamic_cast<ColliderCapsule*>(
				ownColliders_.at(static_cast<int>(COLLIDER_TYPE::CAPSULE)));
			colCap->SetLocalPosTop(COL_CAPSULE_TOP_LOCAL_POS);
			colCap->SetLocalPosDown(COL_CAPSULE_DOWN_LOCAL_POS);
		}
	}
}

void Player::DrawDebug(void)
{
	DrawFormatString(10, 10, GetColor(255, 255, 255), "プレイヤー座標：\n X=%.2f\nY=%.2f\nZ=%.2f",
		transform_.pos.x, transform_.pos.y, transform_.pos.z);
}