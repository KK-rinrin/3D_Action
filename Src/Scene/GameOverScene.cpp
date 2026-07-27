#include <DxLib.h>
#include <cmath>
#include "../Application.h"
#include "../Manager/Camera.h"
#include "../Manager/InputManager.h"
#include "../Manager/ResourceManager.h"
#include "../Manager/SceneManager.h"
#include "GameOverScene.h"

GameOverScene::GameOverScene(void)
	:
	SceneBase(),
	imgGameOver_(-1),
	imgPushSpace_(-1),
	blinkCounter_(0),
	isPushSpaceDraw_(false),
	gameOverPosY_(GAME_OVER_START_Y),
	gameOverVelocityY_(0.0f),
	isGameOverBounceEnd_(false)
{
}

GameOverScene::~GameOverScene(void)
{
}

void GameOverScene::Init(void)
{
	// 固定カメラへ切り替える
	sceMng_.GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);

	// 画像読み込み
	imgGameOver_ = resMng_.Load(
		ResourceManager::SRC::GAME_OVER_IMG).handleId_;
	imgPushSpace_ = resMng_.Load(
		ResourceManager::SRC::TITLE_PUSH_START).handleId_;

	blinkCounter_ = 0;
	isPushSpaceDraw_ = false;
	gameOverPosY_ = GAME_OVER_START_Y;
	gameOverVelocityY_ = 0.0f;
	isGameOverBounceEnd_ = false;
}

void GameOverScene::Update(void)
{
	// GAME OVER画像を落下させ、減衰しながらバウンドさせる
	if (!isGameOverBounceEnd_)
	{
		gameOverVelocityY_ += GAME_OVER_GRAVITY;
		gameOverPosY_ += gameOverVelocityY_;

		// 所定位置へ到達したらY座標を固定して速度を反転する
		if (gameOverPosY_ > GAME_OVER_TARGET_Y)
		{
			gameOverPosY_ = GAME_OVER_TARGET_Y;
			if (std::fabs(gameOverVelocityY_) > GAME_OVER_STOP_VELOCITY)
			{
				gameOverVelocityY_ =
					-gameOverVelocityY_ * GAME_OVER_BOUNCE_DAMP;
			}
			else
			{
				gameOverVelocityY_ = 0.0f;
				isGameOverBounceEnd_ = true;
				isPushSpaceDraw_ = true;
			}
		}
		return;
	}

	// PushSpace画像の点滅
	if (blinkCounter_++ >= PUSH_SPACE_BLINK_INTERVAL)
	{
		isPushSpaceDraw_ = !isPushSpaceDraw_;
		blinkCounter_ = 0;
	}

	auto& input = InputManager::GetInstance();

	// キーボードの決定入力
	bool isDecide = input.IsTrgDown(KEY_INPUT_SPACE);

	// ゲームパッドの決定入力
	if (GetJoypadNum() > 0)
	{
		isDecide = isDecide || input.IsPadBtnTrgDown(
			InputManager::JOYPAD_NO::PAD1,
			InputManager::JOYPAD_BTN::DOWN);
	}

	if (isDecide)
	{
		sceMng_.ChangeScene(SceneManager::SCENE_ID::TITLE);
	}
}

void GameOverScene::Draw(void)
{
	// 背景
	DrawBox(
		0, 0,
		Application::SCREEN_SIZE_X, Application::SCREEN_SIZE_Y,
		GetColor(12, 12, 18), true);

	// ゲームオーバー画像
	DrawRotaGraph(
		Application::SCREEN_SIZE_X / 2,
		static_cast<int>(gameOverPosY_),
		1.0, 0.0,
		imgGameOver_, true);

	// タイトルへ戻る操作を表示
	if (isPushSpaceDraw_)
	{
		DrawRotaGraph(
			Application::SCREEN_SIZE_X / 2,
			Application::SCREEN_SIZE_Y / 2 + 150,
			1.0, 0.0,
			imgPushSpace_, true);
	}
}

void GameOverScene::Release(void)
{
}
