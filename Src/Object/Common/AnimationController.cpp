#include <DxLib.h>
#include "../../Manager/SceneManager.h"
#include "AnimationController.h"

AnimationController::AnimationController(int modelId)
	:
	modelId_(modelId),
	isBlending_(false),
	blendTime_(0.0f),
	priorityType_(-1)
{
}

AnimationController::~AnimationController(void)
{
}

void AnimationController::Add(int type, float speed, const std::string path)
{
	Animation animation;
	animation.model = MV1LoadModel(path.c_str());
	animation.animIndex = -1;
	animation.totalTime =
		MV1GetAnimTotalTime(animation.model, 0);

	Add(type, speed, animation);
}

void AnimationController::AddInFbx(int type, float speed, int animIndex)
{
	Animation animation;
	animation.model = -1;
	animation.animIndex = animIndex;
	animation.totalTime =
		MV1GetAnimTotalTime(modelId_, animation.animIndex);

	Add(type, speed, animation);
}

void AnimationController::Play(int type, bool isLoop, float blendTime)
{

	// 優先されているアニメーションタイプを検出
	int priorityType = -1;
	for (const auto& data : animations_)
	{
		if (!data.second.isPriority) continue;
		priorityType = data.first;
	}

	if (priorityType_ == type)
	{
		// 要求されたアニメーションが優先中の場合のみ何もしない
		return;
	}

	// 要求されたアニメーションをアタッチ
	auto& priAnim = animations_.at(type);
	if (priAnim.attachNo > -1)
	{
		// フェードアウト中の同一アニメーションは再利用せず、先頭から再生する
		MV1DetachAnim(modelId_, priAnim.attachNo);
		priAnim.attachNo = -1;
		priAnim.isPriority = false;
		priAnim.step = 0.0f;
		priAnim.blendRate = 0.0f;
		priAnim.blendStep = 0.0f;
		priAnim.weight = 0.0f;
	}
	priAnim.attachNo = AttachAnim(priAnim);
	priAnim.isPriority = true;
	priAnim.step = 0.0f;
	priAnim.blendStep = 0.0f;

	if (priorityType == -1)
	{
		// 何も再生されていない
		priAnim.blendRate = 1.0f;
	}
	else
	{
		// アニメーションをブレンド中にする
		isBlending_ = true;

		// 現在設定されている優先アニメーションを非優先にする
		auto& oldPri = animations_.at(priorityType);
		oldPri.isPriority = false;
	}

	// 現在のブレンド率を重みとして保存しておく
	for (auto& anim : animations_)
	{
		anim.second.weight = anim.second.blendRate;
	}

	// アニメーションループ
	priAnim.isLoop = isLoop;

	// 更新
	blendTime_ = blendTime;
	priorityType_ = type;

}

void AnimationController::Update(void)
{

	// 経過時間の取得
	float deltaTime = SceneManager::GetInstance().GetDeltaTime();

	// 再生
	auto& priAnim = animations_.at(priorityType_);
	priAnim.step += (deltaTime * priAnim.speed);
	priAnim.blendStep += deltaTime;

	// ブレンド率の計算
	if (isBlending_)
	{

		// 優先アニメーション
		priAnim.blendRate = (priAnim.blendStep / blendTime_);
		if (priAnim.blendRate >= 1.0f)
		{
			// ブレンド完了
			priAnim.blendRate = 1.0f;
			isBlending_ = false;
		}

		// 非優先アニメーション
		float rev = 1.0f - priAnim.blendRate;
		for (auto& anim : animations_)
		{
			if (anim.second.isPriority) continue;
			if (anim.second.attachNo == -1) continue;

			// アニメーション進行
			anim.second.step += (deltaTime * anim.second.speed);
			if (anim.second.step > anim.second.totalTime)
			{
				anim.second.step = anim.second.totalTime;
			}
			// ブレンド率を引いていく
			anim.second.blendRate = anim.second.weight * rev;
		}

		for (auto& anim : animations_)
		{
			if (anim.second.attachNo == -1) continue;
			// アニメーション時間更新
			MV1SetAttachAnimTime(modelId_, anim.second.attachNo, anim.second.step);
			// ブレンド率更新
			MV1SetAttachAnimBlendRate(modelId_, anim.second.attachNo, anim.second.blendRate);
			if (anim.second.blendRate <= 0.0f)
			{
				// デタッチ
				MV1DetachAnim(modelId_, anim.second.attachNo);
				anim.second.attachNo = -1;
				anim.second.step = 0.0f;
				anim.second.blendRate = 0.0f;
				anim.second.blendStep = 0.0f;
				anim.second.weight = 0.0f;
			}
		}

	}
	else
	{

		if (priAnim.step > priAnim.totalTime)
		{
			if (priAnim.isLoop)
			{
				// ループ再生
				priAnim.step = 0.0f;
			}
			else
			{
				// ループしない
				priAnim.step = priAnim.totalTime;
			}
		}

		// ブレンドしていないなら、アニメーション時間を更新するだけでいい
		MV1SetAttachAnimTime(modelId_, priAnim.attachNo, priAnim.step);

	}

}

void AnimationController::Release(void)
{

	// 外部FBXのモデル(アニメーション)解放
	for (const std::pair<int, Animation>& pair : animations_)
	{
		if (pair.second.model != -1)
		{
			MV1DeleteModel(pair.second.model);
		}
	}
	
	// 可変長配列をクリアする
	animations_.clear();
	
}

int AnimationController::GetPlayType(void) const
{
	return priorityType_;
}

bool AnimationController::IsEnd(int animType) const
{
	bool ret = false;

	const auto& anim = animations_.at(animType);

	if (anim.isLoop)
	{
		// ループ設定されているなら、
		// 無条件で終了しないを返す
		return false;
	}


	if (anim.blendRate <= 0.0f)
	{
		// 再生していなかったら終了判定
		return true;
	}
	if (anim.step >= anim.totalTime)
	{
		// 再生時間を過ぎたらtrue
		return true;
	}

	return ret;

}

const AnimationController::Animation& AnimationController::GetPlayAnim(void) const
{
	return animations_.at(priorityType_);
}

void AnimationController::Add(int type, float speed, Animation& animation)
{
	animation.speed = speed;

	if (animations_.count(type) == 0)
	{
		// 追加
		animations_.emplace(type, animation);
	}
}

int AnimationController::AttachAnim(const Animation& animation) const
{

	int attachNo = -1;

	// モデルにアニメーションを付ける
	if (animation.model == -1)
	{
		// モデルと同じファイルからアニメーションをアタッチする
		attachNo = MV1AttachAnim(modelId_, animation.animIndex);
	}
	else
	{
		// 別のモデルファイルからアニメーションをアタッチする
		// DxModelViewerを確認すること(大体0か1)
		int animIdx = 0;
		attachNo = MV1AttachAnim(modelId_, animIdx, animation.model);
	}

	return attachNo;

}
