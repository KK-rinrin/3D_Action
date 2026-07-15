#pragma once
#include <map>
#include <DxLib.h>
#include "InputManager.h"
class InputBinder
{
public:
	// 操作アクションの種類
	enum class ACTION
	{
		MOVE_F,		// 前移動
		MOVE_L,		// 左移動
		MOVE_B,		// 後移動
		MOVE_R,		// 右移動
		ROTX_N,		// カメラ上回転
		ROTX_R,		// カメラ下回転
		ROTY_N,		// カメラ右回転
		ROTY_R,		// カメラ左回転
		DASH,		// ダッシュ
		JUMP,		// ジャンプ
		ATTACK,		// 攻撃
		MAX
	};

	// コンストラクタ
	InputBinder(InputManager::JOYPAD_NO padNo);

	// デストラクタ
	~InputBinder(void);

	// 初期化
	void Init(void);

	// 更新
	void Update(void);

	// ダッシュ
	bool IsDash(void) const { return isDash_; };

	// ジャンプ（入力持続）
	bool IsJump(void) const { return isJump_; };

	// ジャンプ（押した瞬間）
	bool IsTrgJump(void) const { return isTrgJump_; };

	// 攻撃
	bool IsAttack(void) const { return isAttack_; };

	// 入力方向の取得
	const VECTOR& GetDirection(void) const { return dir_; };

	// カメラの回転量の取得
	const VECTOR& GetRotPowCamera(void) const { return rotPowCamera_; };
private:
	// カメラの回転量
	const float ROT_POW_DEG = 2.0f;
	const float ROT_POW_RAD = ROT_POW_DEG * DX_PI_F / 180.0f;

	// 入力マネージャーのインスタンス
	InputManager& inputIns_;

	// 操作アクションとキーボードのマッピング
	std::map<ACTION, int> actionKeyboardMap_;

	// 操作アクションとパッドのマッピング
	std::map<ACTION, int> actionPadMap_;

	// ゲームパッドの番号
	InputManager::JOYPAD_NO padNo_;

	// 入力方向
	VECTOR dir_;

	// カメラの回転量
	VECTOR rotPowCamera_;

	// ダッシュ
	bool isDash_;

	// ジャンプ
	bool isJump_;
	bool isTrgJump_;

	// 攻撃
	bool isAttack_;

	// 更新
	void UpdateKeyboard(void);
	void UpdatePad(void);
};