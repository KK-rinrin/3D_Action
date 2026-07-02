#include "../../../Utility/SchoolUtility.h"
#include "../../../Application.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/SceneManager.h"
#include "../../../Manager/InputManager.h"
#include "../../../Manager/Camera.h"
#include "../../Common/AnimationController.h"
#include "../../Collider/ColliderLine.h"
#include "../../Collider/ColliderCapsule.h"
#include "../Weapon/WeaponBlade.h"
#include "Player.h"

const float WEAPON_HIT_START[] =
{
	23.0f,	// VERTICAL_SLASH
	25.0f,	// HORIZONTAL_SLASH
	26.0f	// SPINNING_SLASH
};

const float WEAPON_HIT_END[] =
{
	27.0f,	// VERTICAL_SLASH
	30.0f,	// HORIZONTAL_SLASH
	34.0f	// SPINNING_SLASH
};

Player::Player()
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
	// カメラの向きに応じた移動方向の取得（拡張しやすい形に整理）
	// 入力の取得
	InputManager& ins = InputManager::GetInstance();
	bool isDash = false;
	moveDir_ = SchoolUtility::VECTOR_ZERO;

	if (GetJoypadNum() == 0)
	{
		if (ins.IsNew(KEY_INPUT_W)) { moveDir_ = SchoolUtility::DIR_F; }
		if (ins.IsNew(KEY_INPUT_S)) { moveDir_ = SchoolUtility::DIR_B; }
		if (ins.IsNew(KEY_INPUT_A)) { moveDir_ = SchoolUtility::DIR_L; }
		if (ins.IsNew(KEY_INPUT_D)) { moveDir_ = SchoolUtility::DIR_R; }
		if (ins.IsNew(KEY_INPUT_RSHIFT)) { isDash = true; }

	}
	else
	{
		// 接続されているゲームパッド１の情報を取得
		InputManager::JOYPAD_IN_STATE padState = ins.GetJPadInputState(InputManager::JOYPAD_NO::PAD1);
		VECTOR pad1dir = ins.GetDirectionXZAKey(padState.AKeyLX, padState.AKeyLY);

		moveDir_.x += pad1dir.x;
		moveDir_.z += pad1dir.z;
		if (ins.IsPadBtnNew(InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::R_TRIGGER))
		{ isDash = true; }
	}
	// カメラ基底を SceneManager 経由で取得（存在しない場合はワールド基準を使う）
	VECTOR camForward = SchoolUtility::DIR_F; // フォールバック前方
	VECTOR camRight = SchoolUtility::DIR_R;   // フォールバック右
	auto camera = SceneManager::GetInstance().GetCamera();
	if (camera != nullptr)
	{
		// カメラ前方向（注視点方向）
		camForward = camera->GetForward();
		// カメラの Y 回転のみの回転から右方向を取得（Y回転だけにすることでXZ平面の右方向を得る）
		camRight = camera->GetQuaRotY().GetRight();
	}

	// XZ 平面に射影して正規化（カメラが傾いていても XZ の進行方向のみを使うため）
	camForward.y = 0.0f;
	camRight.y = 0.0f;
	camForward = SchoolUtility::VNormalize(camForward);
	camRight = SchoolUtility::VNormalize(camRight);


	if (!SchoolUtility::EqualsVZero(moveDir_))
	{
		// カメラ右方向 * inX + カメラ前方向 * inZ
		VECTOR dirX = VScale(camRight, moveDir_.x);
		VECTOR dirZ = VScale(camForward, moveDir_.z);
		moveDir_ = VAdd(dirX, dirZ);
		// 正規化して方向のみを保持
		moveDir_ = SchoolUtility::VNormalize(moveDir_);

		// 移動中
		moveSpeed_ = isDash ? SPEED_DASH : SPEED_MOVE;

		// ジャンプ中はアニメーションを変えない
		if (!isJump_)
		{
			animController_->Play(static_cast<int>(isDash ? ANIM_TYPE::FAST_RUN : ANIM_TYPE::RUN));
		}
	}
	else
	{
		// 停止中
		moveSpeed_ = 0.0f;
		if (!isJump_) animController_->Play(static_cast<int>(ANIM_TYPE::IDLE));
		moveDir_ = SchoolUtility::VECTOR_ZERO;
	}

	// 移動量の計算（既存のフローに合わせ速度のみ使用）
	movePow_ = VScale(moveDir_, moveSpeed_);
}

void Player::ProcessJump(void)
{
	auto& ins = InputManager::GetInstance();
	// 持続ジャンプ処理
	bool isHitKeyNew = ins.IsNew(KEY_INPUT_BACKSLASH)
		|| ins.IsPadBtnNew(
			InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN);
	if (isHitKeyNew)
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
	bool isHitKey = ins.IsTrgDown(KEY_INPUT_BACKSLASH)
		|| ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::DOWN);
	// ジャンプ
	if (isHitKey && !isJump_)
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

	auto& ins = InputManager::GetInstance();

	// 攻撃ボタン
	bool isHitKey = ins.IsTrgDown(KEY_INPUT_COLON) || ins.IsTrgDown(KEY_INPUT_X) || 
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::LEFT);

	if (isHitKey || isTrgDownWhileAttack_)
	{
		// 連続攻撃回数を加算
		attackChain_++;
		ChangeState(STATE::ATTACK);
		return;
	}

	// 連続攻撃記録をリセット
	attackChain_ = 0;
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

	auto animStep = animController_->GetPlayAnim().step;

	auto& ins = InputManager::GetInstance();
	// 攻撃が終わる直前にボタンを押したら次の攻撃を準備
	if ((ins.IsTrgDown(KEY_INPUT_COLON) || ins.IsTrgDown(KEY_INPUT_X)) &&
		animStep > animController_->GetPlayAnim().totalTime - BUTTON_WHILE_ATTACK_STEP)
		isTrgDownWhileAttack_ = true;

	int atkVar = attackChain_ % ATTACK_VARIOUS;

	float start, end;
	// フレーム
	if (isAttackAnim(animController_->GetPlayType())) {
		auto frame = get(toAttackAnim(animController_->GetPlayType()));
		start = frame.start;
		end = frame.end;
	}

	if (animStep >= start && animStep <= end)
	{
		weapon_->SetAllColliderValid(true);
	}
	else
	{
		weapon_->SetAllColliderValid(false);
	}

	// アニメーションの再生終了
	if (animController_->IsEnd())
	{
		ChangeState(STATE::PLAY);
	}
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
	int a = static_cast<int>(state_);

	auto& ins = InputManager::GetInstance();
	int b = static_cast<int>(ins.IsTrgDown(KEY_INPUT_COLON) || 
		ins.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1, InputManager::JOYPAD_BTN::LEFT));

	DrawFormatString(10, 10, GetColor(255, 255, 255), "プレイヤー座標：\n X=%.2f\nY=%.2f\nZ=%.2f",
		transform_.pos.x, transform_.pos.y, transform_.pos.z);


}