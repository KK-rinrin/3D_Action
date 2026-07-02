#pragma once
#include <DxLib.h>
#include "WeaponBase.h"
class Transform;
class ColliderCapsule;

class WeaponBlade : public WeaponBase
{
public:
	// コンストラクタ
	WeaponBlade(const Transform& followTransform, int followFrameId);
	
	// デストラクタ
	~WeaponBlade(void) override;
	
	// 更新
	void Update(void) override;

protected:
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
private:
	// モデルの大きさ
	static constexpr float SCALE = 0.2f;
	
	// 衝突判定用カプセル上部球体
	static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS = { 0.0f, 130.0f, 0.0f };
	
	// 衝突判定用カプセル下部球体
	static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS = { 0.0f, 20.0f, 0.0f };
	
	// 衝突判定用カプセル球体半径
	static constexpr float COL_CAPSULE_RADIUS = 30.0f;
};