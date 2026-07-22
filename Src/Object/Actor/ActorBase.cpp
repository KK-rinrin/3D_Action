#include "../../Manager/ResourceManager.h"
#include "../../Manager/SceneManager.h"
#include "ActorBase.h"

ActorBase::ActorBase(void)
	: 
	resMng_(ResourceManager::GetInstance()),
	scnMng_(SceneManager::GetInstance()),
	transform_()
{
}

ActorBase::~ActorBase(void)
{
}

void ActorBase::Init(void)
{

	// リソースロード
	InitLoad();

	// Transform初期化
	// モデルをロードしないと意味がないので、リソースロードの後に呼び出す
	InitTransform();

	// 衝突判定の初期化
	// Transformの設定が必要になるので、Transform初期化の後に呼び出す
	InitCollider();

	// アニメーションの初期化
	// モデルをロードしないと意味がないので、リソースロードの後に呼び出す
	InitAnimation();

	// 初期化後の個別処理
	InitPost();

}

void ActorBase::Draw(void)
{
	if (transform_.modelId != -1)
	{
		MV1DrawModel(transform_.modelId);
	}

#ifdef _DEBUG
	// 所有しているコライダの描画
	for (const auto& own : ownColliders_)
	{
		own.second->Draw();
	}

	DrawDebug();
#endif // _DEBUG
}

void ActorBase::Release(void)
{
	transform_.Release();

	// 自身のコライダ解放
	for (auto& own : ownColliders_)
	{
		delete own.second;
	}
}

const Transform& ActorBase::GetTransform(void) const
{
	return transform_;
}

const ColliderBase* ActorBase::GetOwnCollider(int key) const
{
	if (ownColliders_.count(key) == 0)
	{
		return nullptr;
	}
	return ownColliders_.at(key);
}

void ActorBase::AddHitCollider(const ColliderBase* hitCollider)
{
	for (const auto& c : hitColliders_)
	{
			if (c == hitCollider)
			{
				return;
			}
	}
	hitColliders_.emplace_back(hitCollider);
}

void ActorBase::ClearHitCollider(void)
{
	hitColliders_.clear();
}

void ActorBase::SetAllColliderValid(bool isValid)
{
	for (const auto& own : ownColliders_)
	{
		own.second->SetValid(isValid);
	}
}

void ActorBase::SetAllColliderKnockBackPow(float power)
{
	for (const auto& own : ownColliders_)
	{
		own.second->SetKnockBackPow(power);
	}
}

void ActorBase::SetColliderValid(int key, bool isValid)
{
	ownColliders_.at(key)->SetValid(isValid);
}

void ActorBase::SetColliderKnockBackPow(int colliderKey, float knockBackPow)
{
	ownColliders_.at(colliderKey)->SetKnockBackPow(knockBackPow);
}
