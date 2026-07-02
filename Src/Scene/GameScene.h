#pragma once
#include "SceneBase.h"

class Stage;
class SkyDome;
class Player;
class EnemyManager;
class ShadowRenderer;

class GameScene : public SceneBase
{

public:
	
	// コンストラクタ
	GameScene(void);

	// デストラクタ
	~GameScene(void) override;

	// 初期化
	void Init(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

	// 解放
	void Release(void) override;

private:
	Stage* stage_;
	SkyDome* skyDome_;
	Player* player_;
	EnemyManager* enemy_;
	ShadowRenderer* shadowRenderer_;
};