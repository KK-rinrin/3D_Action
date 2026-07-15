#include "../Utility/SchoolUtility.h"
#include "InputManager.h"
#include "InputBinder.h"
InputBinder::InputBinder(InputManager::JOYPAD_NO padNo)
	:
	inputIns_(InputManager::GetInstance()),
	padNo_(padNo),
	dir_(SchoolUtility::VECTOR_ZERO),
	isDash_(false),
	isJump_(false),
	isTrgJump_(false),
	isAttack_(false)
{
}
InputBinder::~InputBinder(void)
{
}
void InputBinder::Init(void)
{
	// 操作アクションとキーボードのマッピング
	actionKeyboardMap_.emplace(ACTION::MOVE_F, KEY_INPUT_W);		// 前移動
	actionKeyboardMap_.emplace(ACTION::MOVE_L, KEY_INPUT_A);		// 左移動
	actionKeyboardMap_.emplace(ACTION::MOVE_B, KEY_INPUT_S);		// 後移動
	actionKeyboardMap_.emplace(ACTION::MOVE_R, KEY_INPUT_D);		// 右移動
	actionKeyboardMap_.emplace(ACTION::DASH, KEY_INPUT_RSHIFT);		// ダッシュ
	actionKeyboardMap_.emplace(ACTION::JUMP, KEY_INPUT_BACKSLASH);	// ジャンプ
	actionKeyboardMap_.emplace(ACTION::ATTACK, KEY_INPUT_COLON);	// 攻撃
	actionKeyboardMap_.emplace(ACTION::ROTX_N, KEY_INPUT_UP);		// 上見る
	actionKeyboardMap_.emplace(ACTION::ROTX_R, KEY_INPUT_DOWN);		// 下見る
	actionKeyboardMap_.emplace(ACTION::ROTY_N, KEY_INPUT_RIGHT);	// 右見る
	actionKeyboardMap_.emplace(ACTION::ROTY_R, KEY_INPUT_LEFT);		// 左見る

	// 操作アクションとパッドのマッピング
	actionPadMap_.emplace(ACTION::DASH,
		static_cast<int>(InputManager::JOYPAD_BTN::R_TRIGGER));
	actionPadMap_.emplace(ACTION::JUMP,
		static_cast<int>(InputManager::JOYPAD_BTN::DOWN));
	actionPadMap_.emplace(ACTION::ATTACK,
		static_cast<int>(InputManager::JOYPAD_BTN::LEFT));
}
void InputBinder::Update(void)
{
	// キーボード
	UpdateKeyboard();

	// ゲームパッドが接続数で処理を分ける
	if (GetJoypadNum() > 0)
	{
		// ゲームパッド
		UpdatePad();
	}
}
void InputBinder::UpdateKeyboard(void)
{
	// 入力方向のリセット
	dir_ = SchoolUtility::VECTOR_ZERO;

	// 移動方向
	if (inputIns_.IsNew(
		actionKeyboardMap_.at(ACTION::MOVE_F))) {
		dir_ = VAdd(dir_,SchoolUtility::DIR_F);
	}
	if (inputIns_.IsNew(
		actionKeyboardMap_.at(ACTION::MOVE_L))) {
		dir_ = VAdd(dir_,SchoolUtility::DIR_L);
	}
	if (inputIns_.IsNew(
		actionKeyboardMap_.at(ACTION::MOVE_B))) {
		dir_ = VAdd(dir_,SchoolUtility::DIR_B);
	}
	if (inputIns_.IsNew(
		actionKeyboardMap_.at(ACTION::MOVE_R))) {
		dir_ = VAdd(dir_,SchoolUtility::DIR_R);
	}
	if (VSize(dir_) > SchoolUtility::kEpsilonNormalSqrt) dir_ = VNorm(dir_);

	// カメラ回転量
	if (inputIns_.IsNew(
		actionKeyboardMap_.at(ACTION::ROTY_N)))
	{
		// 右回転
		rotPowCamera_.y += ROT_POW_RAD;
	}
	if (inputIns_.IsNew(
		actionKeyboardMap_.at(ACTION::ROTY_R)))
	{
		// 左回転
		rotPowCamera_.y -= ROT_POW_RAD;
	}
	if (inputIns_.IsNew(
		actionKeyboardMap_.at(ACTION::ROTX_N)))
	{
		// 上回転
		rotPowCamera_.x += ROT_POW_RAD;
	}
	if (inputIns_.IsNew(
		actionKeyboardMap_.at(ACTION::ROTX_R)))
	{
		// 下回転
		rotPowCamera_.x -= ROT_POW_RAD;
	}

	// ダッシュ
	isDash_ = inputIns_.IsNew(actionKeyboardMap_.at(ACTION::DASH));

	// ジャンプ
	isJump_ = inputIns_.IsNew(actionKeyboardMap_.at(ACTION::JUMP));
	isTrgJump_ = inputIns_.IsTrgDown(actionKeyboardMap_.at(ACTION::JUMP));

	// 攻撃
	isAttack_ = inputIns_.IsTrgDown(actionKeyboardMap_.at(ACTION::ATTACK));
}
void InputBinder::UpdatePad(void)
{
	// 接続されているゲームパッド１の情報を取得
	InputManager::JOYPAD_IN_STATE padState = inputIns_.GetJPadInputState(padNo_);

	// アナログキーの入力値から方向を取得
	dir_ = inputIns_.GetDirectionXZAKey(padState.AKeyLX, padState.AKeyLY);

	// アナログキーの入力値からカメラ回転量を取得(x左右 z前後)
	rotPowCamera_ = SchoolUtility::VECTOR_ZERO;
	
	VECTOR rot = inputIns_.GetDirectionXZAKey(padState.AKeyRX, padState.AKeyRY);
	rotPowCamera_.y = rot.x * ROT_POW_RAD;
	rotPowCamera_.x = rot.z * ROT_POW_RAD;

	// ダッシュ
	isDash_ = inputIns_.IsPadBtnNew(padNo_,
		static_cast<InputManager::JOYPAD_BTN>(actionPadMap_.at(ACTION::DASH)));

	// ジャンプ
	isJump_ = inputIns_.IsPadBtnNew(padNo_,
		static_cast<InputManager::JOYPAD_BTN>(actionPadMap_.at(ACTION::JUMP)));

	isTrgJump_ = inputIns_.IsPadBtnTrgDown(padNo_,
		static_cast<InputManager::JOYPAD_BTN>(actionPadMap_.at(ACTION::JUMP)));

	// 攻撃
	isAttack_ = inputIns_.IsPadBtnTrgDown(padNo_,
		static_cast<InputManager::JOYPAD_BTN>(actionPadMap_.at(ACTION::ATTACK)));
}

