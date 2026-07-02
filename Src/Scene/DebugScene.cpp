#include <fstream>
#include <DxLib.h>
#include "../Common/Vector2.h"
#include "../Manager/InputManager.h"
#include "../Manager/SceneManager.h"
#include "../Manager/Camera.h"
#include "../Object/Actor/Stage.h"
#include "../Object/Collider/ColliderModel.h"
#include "DebugScene.h"

DebugScene::DebugScene(void)
	:
	SceneBase(),
	stage_(nullptr)
{
}

DebugScene::~DebugScene(void)
{
}
void DebugScene::Init(void)
{
	// ステージ生成
	stage_ = new Stage();
	stage_->Init();

	// カメラの追従設定
	Camera* camera = sceMng_.GetCamera();
	camera->ChangeMode(Camera::MODE::FREE);
}

void DebugScene::Update(void)
{
	// ステージ更新
	stage_->Update();
	// デパッグポイントの配置
	PlaceDebugPoint();
}

void DebugScene::Draw(void)
{
	// ステージ描画
	stage_->Draw();

	// デバッグポイント群を球体描画
	int y = 20;
	for (const auto& point : points_)
	{
		DrawSphere3D(
			point,
			30.0f,
			16,
			GetColor(255, 0, 0),
			GetColor(255, 0, 0),
			false);
		DrawFormatString(20, y,
			0x000000, "座標(%.2f, %.2f, %.2f)",
			point.x, point.y, point.z);
		y += 20;
	}
}

void DebugScene::Release(void)
{
	// ステージ解放
	stage_->Release();
	delete stage_;

	// デバッグポイント群
	points_.clear();
}

void DebugScene::PlaceDebugPoint(void)
{
	const auto& ins = InputManager::GetInstance();

	// クリックした場所にデバッグポイントを配置
	if (ins.IsTrgMouseLeft())
	{
		// マウス座標の取得
		Vector2 mousePos = ins.GetMousePos();

		// スクリーン座標をVECTOR構造体に変換
		VECTOR screenPos = VECTOR();
		screenPos.x = static_cast<float>(mousePos.x);
		screenPos.y = static_cast<float>(mousePos.y);

		// zが1.0fでカメラの最奥になる
		screenPos.z = 1.0f;

		// マウスの２Ｄ座標から３D座標へ変換
		VECTOR worldPos = ConvScreenPosToWorldPos(screenPos);

		// ステージのモデルコライダを取得
		const ColliderBase* collider = stage_->GetOwnCollider(
			static_cast<int>(Stage::COLLIDER_TYPE::MODEL));
		if (collider == nullptr)
		{
			return;
		}

		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(collider);

		// カメラ情報を取得
		const auto& camera = SceneManager::GetInstance().GetCamera();

		// カメラの位置からカメラ最奥のワールド座標へ向けてレイを飛ばす
		auto hit = MV1CollCheck_Line(
			colliderModel->GetFollow()->modelId, -1,
			camera->GetPos(),
			worldPos
		);

		if (hit.HitFlag)
		{
			// 衝突地点をデバッグポイント群に追加
			points_.push_back(hit.HitPosition);
		}
	}

	// 右クリックで最後のデバッグポイントを削除
	if (ins.IsTrgMouseRight())
	{
		if (points_.size() > 0)
		{
			points_.pop_back();
		}
	}

	if (ins.IsTrgDown(KEY_INPUT_SPACE))
	{
		// デバッグポイントの保存
		SavePoints();
	}

}

void DebugScene::SavePoints(void)
{
	// ofs = output file stream
	std::ofstream ofs("Data/Csv/PointSave.txt");
	if (!ofs) {
		return;
	}

	// 形式: x y z
	for (const VECTOR& p : points_) {
		ofs << p.x << " " << p.y << " " << p.z << "\n";
	}

	ofs.close();
}