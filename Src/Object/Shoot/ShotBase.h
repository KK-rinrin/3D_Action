#pragma once
#include <vector>
#include <DxLib.h>
#include "../../Utility/SchoolUtility.h"
class Transform;

// 球弾データ（汎用）
struct SphereShot
{
	VECTOR pos; // 座標
	float r;    // 半径
	VECTOR vel; // 速度（ワールド空間）
	float life; // 残り寿命（秒）
};

class ShotBase
{
public:
	ShotBase();
	~ShotBase();

	// 弾生成（位置、速度、半径、寿命）
	void SpawnShot(const VECTOR& pos, const VECTOR& vel, float radius, float life = 5.0f);

	// 更新（dt を外部から渡す）
	void Update(float dt);

	// 描画（DrawSphere3D を利用）
	void Draw() const;

	// 全弾削除
	void Clear();

	// 参照取得
	const std::vector<SphereShot>& GetShots() const { return shots_; }

private:
	std::vector<SphereShot> shots_;
};