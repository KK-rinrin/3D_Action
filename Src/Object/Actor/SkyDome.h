#pragma once
#include "ActorBase.h"

class SkyDome : public ActorBase
{
public:
	SkyDome();
	~SkyDome() override;

	void Update() override;
	void Draw() override;
	void Release() override;

private:
	static constexpr float SCALE = 100.0f;
	static constexpr VECTOR SCALES = { SCALE, SCALE, SCALE };
	static constexpr VECTOR INIT_POS = { 0.0f, 0.0f, 0.0f };
	static constexpr VECTOR ROT = { 0.0f, 180.0f * (DX_PI / 180.0f), 0.0f };
	static constexpr VECTOR TURN = {0.0f, 0.005f * (DX_PI / 180.0f), 0.0f};

	// リソースロード
	void InitLoad(void) override;

	// 大きさ、回転、座標の初期化
	void InitTransform(void) override;

	// 衝突判定の初期化
	void InitCollider(void) override;

	// アニメーションの初期化
	void InitAnimation(void) override;

	// 初期化後の個別処理
	void InitPost(void) override;
};