#include "../../../Manager/ResourceManager.h"
#include "../../../Manager/SceneManager.h"
#include "UISurprise.h"
UISurprise::UISurprise(
	Transform* follow, float localHeight,
	float bounceHeight, float imgSize)
	:
	follow_(follow),
	localHeight_(localHeight),
	bounceHeight_(bounceHeight),
	imgSize_(imgSize)
{
}

UISurprise::~UISurprise(void)
{
}

void UISurprise::Update(void)
{
	if (!active_) return;
	// 演出進行
	step_ += scnMng_.GetDeltaTime();
	if (step_ > DURATION)
	{
		step_ = 1.0f;
		active_ = false;
	}
	// バウンスイージング
	float t = 1.0f;
	if (step_ < TIME_BOUNCE)
	{
		t = EaseOutBounce(step_ / TIME_BOUNCE);
	}
	// 高さ調整
	float easingOffset = t * bounceHeight_;
	// 追従
	transform_.pos = follow_->pos;
	// UIのベース座標にイージング移動量を加える
	transform_.pos.y += localHeight_ + easingOffset;
}

void UISurprise::Draw(void)
{
	if (!active_) return;
	// ビックリマーク描画
	DrawBillboard3D(
		transform_.pos,
		0.5f, 0.5f,
		imgSize_, 0.0f, img_, true);
}

void UISurprise::Start(void)
{
	SetActive(true);
	step_ = 0.0f;
}

void UISurprise::InitLoad(void)
{
	// UI：ビックリマーク
	img_ = resMng_.Load(ResourceManager::SRC::UI_SURPRISE).handleId_;
}

void UISurprise::InitTransform(void)
{
}

void UISurprise::InitPost(void)
{
}

float UISurprise::EaseOutBounce(float t)
{
	// Robert Penner（ロバート・ペナー）氏による
	// バウンス(跳ね返り)のイージング
	float ret = 0.0f;
	// Step①～④の進行比率
	const float SPLIT = 2.75f;
	const float RATE1 = 1.0f / SPLIT;
	const float RATE2 = 2.0f / SPLIT;
	const float RATE3 = 2.5f / SPLIT;
	if (t < RATE1)
	{
		// Step①：大ジャンプ
		ret = POW_BOUNCE * t * t;
	}
	else if (t < RATE2)
	{
		// Step②：跳ね返り
		t -= 1.2f / 2.75f;
		ret = POW_BOUNCE * t * t + 0.80f;
	}
	else if (t < RATE3)
	{
		// Step③：小さな跳ね
		t -= 2.0f / 2.75f;
		ret = POW_BOUNCE * t * t + 0.95f;
	}
	else
	{
		// Step④：微小バウンド
		t -= 2.3f / 2.75f;
		ret = POW_BOUNCE * t * t + 0.99f;
	}
	return ret;
}