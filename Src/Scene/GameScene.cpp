#include <DxLib.h>
#include <vector>
#include "../Manager/SceneManager.h"
#include "../Manager/InputManager.h"
#include "../Manager/Camera.h"
#include "GameScene.h"
#include "../Object/Actor/Stage.h"
#include "../Object/Actor/SkyDome.h"
#include "../Object/Actor/Character/Player.h"
#include "../Object/Actor/Weapon/WeaponBlade.h"
#include "../Manager/EnemyManager.h"
#include "../Object/Renderer/ShadowRenderer.h"

GameScene::GameScene(void)
	:
	SceneBase()
	, stage_(nullptr)
	, skyDome_(nullptr)
	, player_(nullptr)
	, enemy_(nullptr)
	, shadowRenderer_(nullptr)
{
}

GameScene::~GameScene(void)
{
}

void GameScene::Init(void)
{
	if (!stage_) stage_ = new Stage();
	stage_->Init();

	if (!skyDome_) skyDome_ = new SkyDome();
	skyDome_->Init();

	if (!player_) player_ = new Player();
	player_->Init();

	if (!enemy_) enemy_ = new EnemyManager();
	enemy_->Init();
	enemy_->SetTargetTransform(&player_->GetTransform());

	if (!shadowRenderer_) shadowRenderer_ = new ShadowRenderer();

	// カメラモード
	sceMng_.GetCamera()->SetFollow(&player_->GetTransform());
	sceMng_.GetCamera()->ChangeMode(Camera::MODE::FOLLOW);

	// ステージモデルのコライダーをプレイヤーに登録
	const ColliderBase* stageCollider =
		stage_->GetOwnCollider(static_cast<int>(Stage::COLLIDER_TYPE::MODEL));
	player_->AddHitCollider(stageCollider);

	// カメラに登録
	sceMng_.GetCamera()->AddHitCollider(stageCollider);

	// エネミーに登録
	enemy_->AddHitCollider(stageCollider);

	// プレイヤーのカプセルコライダをエネミーに登録
	const ColliderBase* playerCapsule = player_->GetOwnCollider(
		static_cast<int>(CharacterBase::COLLIDER_TYPE::CAPSULE));
	if (playerCapsule != nullptr)
	{
		enemy_->AddHitCollider(playerCapsule);
	}

	// プレイヤー武器のカプセルコライダをエネミーに登録
	const ColliderBase* weaponCapsule = player_->GetWeapon()->
		GetOwnCollider(static_cast<int>(WeaponBlade::COLLIDER_TYPE::CAPSULE));
	if (weaponCapsule != nullptr)
	{
		enemy_->AddHitCollider(weaponCapsule);
	}
}

void GameScene::Update(void)
{
	// シーン遷移
	auto const& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_ESCAPE))
	{
		sceMng_.ChangeScene(SceneManager::SCENE_ID::TITLE);
	}

	skyDome_->Update();
	stage_->Update();
	player_->Update();
	enemy_->Update();
}

void GameScene::Draw(void)
{
	skyDome_->Draw();

	if (shadowRenderer_ != nullptr && shadowRenderer_->IsEnabled())
	{
		std::vector<const ColliderBase*> shadowColliders;
		const ColliderBase* stageCollider =
			stage_->GetOwnCollider(static_cast<int>(Stage::COLLIDER_TYPE::MODEL));
		if (stageCollider != nullptr)
		{
			shadowColliders.emplace_back(stageCollider);
		}

		std::vector<const Transform*> casterTransforms;
		casterTransforms.emplace_back(&player_->GetTransform());
		for (const auto& enemy : enemy_->GetEnemies())
		{
			casterTransforms.emplace_back(&enemy->GetTransform());
		}

		shadowRenderer_->BuildShadowMap(player_->GetTransform(), shadowColliders, casterTransforms);
		shadowRenderer_->BeginApplyShadowMap();
		player_->Draw();
		stage_->Draw();
		enemy_->Draw();
		shadowRenderer_->EndApplyShadowMap();
		return;
	}

	player_->Draw();
	stage_->Draw();
	enemy_->Draw();
}

void GameScene::Release(void)
{
	skyDome_->Release();
	delete skyDome_;
	skyDome_ = nullptr;

	stage_->Release();
	delete stage_;
	stage_ = nullptr;

	player_->Release();
	delete player_;
	player_ = nullptr;

	enemy_->Release();
	delete enemy_;
	enemy_ = nullptr;

	if (shadowRenderer_ != nullptr)
	{
		shadowRenderer_->Release();
		delete shadowRenderer_;
		shadowRenderer_ = nullptr;
	}
}