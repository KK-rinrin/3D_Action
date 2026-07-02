#pragma once
#include "ActorBase.h"
#include <string>

class Stage : public ActorBase
{
public:
	// 衝突判定種別
	enum class COLLIDER_TYPE
	{
		MODEL = 0,
		MAX,
	};

	Stage();
	~Stage() override;

	void Update() override;
	void Release() override;

private:
	// 除外フレーム名称
	const std::vector<std::string> EXCLUDE_FRAME_NAMES = {
	"Mush", "Grass",
	};

	// 対象フレーム
	const std::vector<std::string> TARGET_FRAME_NAMES = {
	"Ground",
	};

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