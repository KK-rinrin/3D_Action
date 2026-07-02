#include <DxLib.h>
#include "Stage.h"
#include "../../Manager/ResourceManager.h"
#include "../Collider/ColliderModel.h"

Stage::Stage()
	: ActorBase()
{
}

Stage::~Stage()
{
}

void Stage::Update()
{
	transform_.Update();
}

void Stage::Release()
{
}

void Stage::InitLoad()
{
	// モデルの読み込み
	transform_.SetModel(resMng_.Load(ResourceManager::SRC::MAIN_STAGE).handleId_);
}

void Stage::InitTransform()
{
	transform_.scl = { 1.0f, 1.0f, 1.0f };
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Identity();
	transform_.pos = { 0.0f, -100.0f, 0.0f };
	transform_.Update();
}

void Stage::InitCollider()
{
	// DxLib側の衝突情報セットアップ
	MV1SetupCollInfo(transform_.modelId);
	// モデルのコライダ
	ColliderModel* colModel =
		new ColliderModel(ColliderBase::TAG::STAGE, &transform_);
	for (const std::string& name : EXCLUDE_FRAME_NAMES)
	{
		colModel->AddExcludeFrameIds(name);
	}
	for (const std::string& name : TARGET_FRAME_NAMES)
	{
		colModel->AddTargetFrameIds(name);
	}
	ownColliders_.emplace(static_cast<int>(COLLIDER_TYPE::MODEL), colModel);

}

void Stage::InitAnimation()
{
}

void Stage::InitPost()
{
}