#pragma once
#include "../ActorBase.h"
class AnimationController;

class CharacterBase : public ActorBase
{
public:

	// 衝突判定種別
	enum class COLLIDER_TYPE
	{
		LINE,
		CAPSULE,
		CONE,
		MAX,
	};

	// コンストラクタ
	CharacterBase(void);
	// デストラクタ
	virtual ~CharacterBase(void) override;

	// 更新
	virtual void Update(void) override;
	// 描画
	virtual void Draw(void) override;
	// 解放
	virtual void Release(void) override;

protected:
	// 最大落下速度
	static constexpr float MAX_FALL_SPEED = -30.0f;
	// 衝突時の押し戻し試行回数
	static constexpr int CNT_TRY_COLLISION = 20;
	// 衝突時の押し戻し量
	static constexpr float COLLISION_BACK_DIS = 1.0f;


	// 初期化系
	// 丸影画像
	int imgShadow_;
	// リソースロード
	virtual void InitLoad(void) override;
	// 大きさ、回転、座標の初期化
	void InitTransform(void) override = 0;
	// 衝突判定の初期化
	void InitCollider(void) override = 0;
	// アニメーションの初期化
	void InitAnimation(void) override = 0;
	// 初期化後の個別処理
	void InitPost(void) override = 0;

	// ジャンプ判定
	bool isJump_;

	// このフレームで着地したか
	bool isTrgLanding_;
	// ジャンプの入力受付時間
	float stepJump_;

	// 更新系
	virtual void UpdateProcess(void) = 0;
	virtual void UpdateProcessPost(void) = 0;
	// 移動方向に応じた遅延回転
	void DelayRotate(void);

	// 移動方向
	VECTOR moveDir_;
	// 移動スピード
	float moveSpeed_;
	// 移動量
	VECTOR movePow_;

	// 移動前の座標
	VECTOR prevPos_;
	// ジャンプ量
	VECTOR jumpPow_;
	// 重力計算
	void CalcGravityPow(void);
	
	AnimationController* animController_;

	// 衝突判定
	virtual void CollisionReserve(void) {}
	void Collision(void);
	void CollisionGravity(void);
	void CollisionCapsule(void);

	// 各キャラごとの衝突処理
	virtual void CollisionPost(void) = 0;

	// 丸影描画
	void DrawShadow(void);
};