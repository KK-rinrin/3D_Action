#pragma once
#include "UIBase.h"
class Transform;
class UISurprise : public UIBase
{
public:
	// コンストラクタ
	UISurprise(
		Transform * follow, float localHeight,
		float bounceHeight, float imgSize);
	// デストラクタ
	~UISurprise(void) override;

	// 更新
	void Update(void) override;

	// 描画
	void Draw(void) override;

	// 描画とイージング開始
	void Start(void);

protected:
	// リソースロード
	void InitLoad(void) override;
	// 大きさ、回転、座標の初期化
	void InitTransform(void) override;
	// 衝突判定の初期化
	void InitCollider(void) override {}
	// アニメーションの初期化
	void InitAnimation(void) override {}
	// 初期化後の個別処理
	void InitPost(void) override;

private:
	// 演出総時間
	static constexpr float DURATION = 2.0f;
	// バウンス演出時間
	static constexpr float TIME_BOUNCE = 0.6f;
	// バウンス強度
	static constexpr float POW_BOUNCE = 9.0f;

	// 追従対象
	Transform* follow_;

	// 画像の大きさ
	float imgSize_;

	// 追従対象からの相対位置(高さ)
	float localHeight_;

	// バウンス幅(高さ)
	float bounceHeight_;

	// イージング関数(バウンスアウト)
	float EaseOutBounce(float t);
};