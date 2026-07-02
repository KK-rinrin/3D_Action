#pragma once
#include "SceneBase.h"
#include "../Object/Common/Transform.h"

class AnimationController;
class SkyDome;

class TitleScene : public SceneBase
{
	static constexpr int PUSH_START_BLINK_INTERVAL = 30;

	static constexpr VECTOR SMALL_PLANET_SCALE = { 0.7f, 0.7f, 0.7f };
	static constexpr VECTOR CHARA_SCALE = { 0.4f, 0.4f,0.4f };

	static enum CHARA_ANIM
	{ 
		RUN,
	};

public:

	// コンストラクタ
	TitleScene(void);

	// デストラクタ
	~TitleScene(void) override;

	// 初期化
	void Init(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

	// 解放
	void Release(void) override;

private:

	// タイトル画像のハンドル
	int imgTitle_;

	int imgPushSpace_;

	int BlinkCounter_;
	bool isPushStartDraw_;

	Transform bigPlanet_;
	Transform spherePlanet_;
	SkyDome* skyDome_;

	Transform character_;
	AnimationController* animController_;
};
