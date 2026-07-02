#pragma once
#include <vector>
#include <DxLib.h>
#include "SceneBase.h"
class Stage;
class DebugScene : public SceneBase
{
public:
	// コンストラクタ
	DebugScene(void);
	// デストラクタ
	~DebugScene(void) override;

	// 初期化
	void Init(void) override;
	// 更新
	void Update(void) override;
	// 描画
	void Draw(void) override;
	// 解放
	void Release(void) override;

private:
	// ステージ
	Stage* stage_;

	// デバッグポイント群
	std::vector<VECTOR> points_;

	// デバッグポイントの配置
	void PlaceDebugPoint(void);

	// デバッグポイントの保存
	void SavePoints(void);
};