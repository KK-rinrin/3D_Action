#pragma once
#include "SceneBase.h"

class GameOverScene : public SceneBase
{
private:
	// PushSpace画像の点滅間隔
	static constexpr int PUSH_SPACE_BLINK_INTERVAL = 30;
	// GAME OVER画像の登場開始Y座標
	static constexpr float GAME_OVER_START_Y = -200.0f;
	// GAME OVER画像の停止Y座標
	static constexpr float GAME_OVER_TARGET_Y = 250.0f;
	// 落下重力
	static constexpr float GAME_OVER_GRAVITY = 2.0f;
	// バウンド時の速度減衰率
	static constexpr float GAME_OVER_BOUNCE_DAMP = 0.5f;
	// バウンド停止判定速度
	static constexpr float GAME_OVER_STOP_VELOCITY = 2.0f;

public:
	// コンストラクタ
	GameOverScene(void);
	// デストラクタ
	~GameOverScene(void) override;

	// 初期化
	void Init(void) override;
	// 更新
	void Update(void) override;
	// 描画
	void Draw(void) override;
	// 解放
	void Release(void) override;

private:
	// ゲームオーバー画像
	int imgGameOver_;
	// PushSpace画像
	int imgPushSpace_;
	// 点滅カウンター
	int blinkCounter_;
	// PushSpace画像を表示するか
	bool isPushSpaceDraw_;
	// GAME OVER画像の表示Y座標
	float gameOverPosY_;
	// GAME OVER画像のY方向速度
	float gameOverVelocityY_;
	// GAME OVER画像の登場演出が終了したか
	bool isGameOverBounceEnd_;
};
