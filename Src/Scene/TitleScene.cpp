#include <DxLib.h>
#include "../Utility/SchoolUtility.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Manager/ResourceManager.h"
#include "TitleScene.h"
#include "../Application.h"
#include "../Object/Common/AnimationController.h"
#include "../Object/Actor/SkyDome.h"

TitleScene::TitleScene(void)
	:
	SceneBase()
	, imgTitle_(-1)
	, imgPushSpace_(-1)
	, BlinkCounter_(0)
	, isPushStartDraw_(true)
{
}

TitleScene::~TitleScene(void)
{
}

void TitleScene::Init(void)
{

	// 定点カメラ
	sceMng_.GetCamera()->ChangeMode(Camera::MODE::FIXED_POINT);

	// 画像読み込み
	imgTitle_ = resMng_.Load(ResourceManager::SRC::TITLE_IMG).handleId_;
	imgPushSpace_ = resMng_.Load(ResourceManager::SRC::TITLE_PUSH_START).handleId_;

	// モデル読み込み
	bigPlanet_.SetModel(resMng_.Load(ResourceManager::SRC::BIG_PLANET).handleId_);
	bigPlanet_.scl = SchoolUtility::VECTOR_ONE;
	bigPlanet_.quaRot = Quaternion::Identity();
	bigPlanet_.quaRotLocal = Quaternion::Identity();
	bigPlanet_.pos = SchoolUtility::VECTOR_ZERO;
	bigPlanet_.Update();

	spherePlanet_.SetModel(resMng_.Load(ResourceManager::SRC::SPHERE_PLANET).handleId_);
	spherePlanet_.scl = SMALL_PLANET_SCALE;
	spherePlanet_.quaRot = Quaternion::Identity();
	spherePlanet_.quaRotLocal = Quaternion::Identity();
	spherePlanet_.quaRotLocal = Quaternion::Euler({ 0.0f, SchoolUtility::Deg2RadF(-90.0f), 0.0f });
	spherePlanet_.pos = { -250.0f, -100.0f, -100.0f };
	spherePlanet_.Update();

	character_.SetModel(resMng_.Load(ResourceManager::SRC::PLAYER).handleId_);
	character_.scl = CHARA_SCALE;
	character_.quaRot = Quaternion::Euler({
		0.0f, SchoolUtility::Deg2RadF(-90.0f), 0.0f });
	character_.quaRotLocal = Quaternion::Euler({
		0.0f, SchoolUtility::Deg2RadF(180.0f), 0.0f });
	character_.pos = { -250.0f, -32.0f, -105.0f };
	character_.Update();

	if (!skyDome_) skyDome_ = new SkyDome();
	skyDome_->Init();

	// アニメーション
	std::string path = Application::PATH_MODEL + "Player/";
	animController_ = new AnimationController(character_.modelId);
	animController_->Add(CHARA_ANIM::RUN, 30.0f, path + "Run.mv1");
	animController_->Play(CHARA_ANIM::RUN);

	BlinkCounter_ = 0;
	isPushStartDraw_ = true;

}

void TitleScene::Update(void)
{
	// 「Push Start」の点滅
	if (BlinkCounter_++ >= PUSH_START_BLINK_INTERVAL)
	{
		isPushStartDraw_ = !isPushStartDraw_;
		BlinkCounter_ = 0;
	}

	// シーン遷移
	auto const& ins = InputManager::GetInstance();
	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		sceMng_.ChangeScene(SceneManager::SCENE_ID::GAME);
	}

	skyDome_->Update();

	// ローカル回転
	// 回転
	// スフィア惑星(小さな惑星)をZ軸回転させる
	Quaternion rotZ = Quaternion::Euler({ 0.0f, 0.0f, SchoolUtility::Deg2RadF(-1.0f) });
	spherePlanet_.quaRotLocal = rotZ.Mult(spherePlanet_.quaRotLocal);
	spherePlanet_.Update();

	animController_->Update();
	character_.Update();




}

void TitleScene::Draw(void)
{
	// 描画順:
	// 描画（不透明）
	// 描画（透明）
	// ポストエフェクト
	// UI描画
	// ポストエフェクト

	// モデルの描画
	skyDome_->Draw();
	MV1DrawModel(bigPlanet_.modelId);
	MV1DrawModel(spherePlanet_.modelId);
	MV1DrawModel(character_.modelId);

	// UI描画
	DrawRotaGraph(
		Application::SCREEN_SIZE_X / 2,
		Application::SCREEN_SIZE_Y / 2 - 100,
		1.0f, 0.0f,
		imgTitle_, true
	);

	if (isPushStartDraw_)
	{
		DrawRotaGraph(
			Application::SCREEN_SIZE_X / 2,
			Application::SCREEN_SIZE_Y / 2 + 150,
			1.0f, 0.0f,
			imgPushSpace_, true
		);
	}
}

void TitleScene::Release(void)
{
	skyDome_->Release();
	delete skyDome_;
	skyDome_ = nullptr;
}
