#include "CharacterBase.h"
#include "../../Common/AnimationController.h"
#include "../../../Application.h"
#include "../../../Manager/SceneManager.h"
#include "../../../Manager/ResourceManager.h"
#include "../../../Utility/SchoolUtility.h"
#include "../../Collider/ColliderLine.h"
#include "../../Collider/ColliderModel.h"
#include "../../Collider/ColliderCapsule.h"
#include "../../Renderer/ShadowRenderer.h"

CharacterBase::CharacterBase(void)
	:
	ActorBase(),
	animController_(nullptr),
	moveDir_(SchoolUtility::VECTOR_ZERO),
	moveSpeed_(0.0f),
	movePow_(SchoolUtility::VECTOR_ZERO),
	isJump_(false),
	stepJump_(0.0f),
	jumpPow_(SchoolUtility::VECTOR_ZERO),
	imgShadow_(-1),
	prevPos_(SchoolUtility::VECTOR_ZERO)

{
}
CharacterBase::~CharacterBase(void)
{
}

void CharacterBase::Update(void)
{
	// 移動前座標を更新
	prevPos_ = transform_.pos;

	// 各キャラクターごとの更新処理
	UpdateProcess();

	// 移動方向に応じた遅延回転
	DelayRotate();

	// 重力による移動量
	CalcGravityPow();

	// 衝突判定前準備
	CollisionReserve();

	// 衝突判定
	Collision();

	// 衝突判定後処理(派生クラスの個別処理)
	CollisionPost();

	// モデル制御更新
	transform_.Update();

	// アニメーション再生
	animController_->Update();

	// 各キャラクターごとの更新後処理
	UpdateProcessPost();

}

void CharacterBase::Draw(void)
{
	// 基底クラスの描画処理
	ActorBase::Draw();

	// 影の描画
	if (!ShadowRenderer::USE_REAL_SHADOW) DrawShadow();
}

void CharacterBase::Release(void)
{
	// アニメーションコントローラー解放
	if (animController_ != nullptr)
	{
		delete animController_;
		animController_ = nullptr;
	}

	// 基底クラスの開放処理を呼ぶ
	ActorBase::Release();
}

void CharacterBase::InitLoad(void)
{
	// 丸影画像
	if (!ShadowRenderer::USE_REAL_SHADOW)
		imgShadow_ = resMng_.Load(ResourceManager::SRC::PLAYER_SHADOW).handleId_;
}

void CharacterBase::DelayRotate(void)
{
	// moveDir_ がほぼゼロ（停止）なら回転補間しない（向きを保持する）
	if (SchoolUtility::SqrMagnitude(moveDir_) <= SchoolUtility::kEpsilonNormalSqrt)
	{
		return;
	}

	// 目標回転（移動方向を向く）
	Quaternion goalRot = Quaternion::LookRotation(moveDir_);
	// 補間して滑らかに回転させる
	transform_.quaRot =
		Quaternion::Slerp(transform_.quaRot, goalRot, 0.2f);
}

void CharacterBase::CalcGravityPow(void)
{
	// 重力方向
	VECTOR dirGravity = SchoolUtility::DIR_D;
	// 重力の強さ
	float gravity1 = Application::GetInstance().GetGravityPow();
	float gravityPow = gravity1 * scnMng_.GetDeltaTime();
	// 重力
	VECTOR gravity = VScale(dirGravity, gravityPow);
	jumpPow_ = VAdd(jumpPow_, gravity);
}

void CharacterBase::Collision(void)
{
	//// ジャンプによるXZ移動の勢い
	//movePow_ = VAdd(movePow_, jumpMomentum_);
	//jumpMomentum_ =
	//	SchoolUtility::Lerp(jumpMomentum_, SchoolUtility::VECTOR_ZERO, 0.1f);

	// 移動処理
	transform_.pos = VAdd(transform_.pos, movePow_);

	// 衝突(カプセル)
	CollisionCapsule();

	// ジャンプ量を加算
	transform_.pos = VAdd(transform_.pos, jumpPow_);

	// 衝突(重力)
	CollisionGravity();
}
void CharacterBase::CollisionGravity(void)
{
	// このフレームで着地したか判定をリセット
	isTrgLanding_ = false;

	// 線分コライダ
	int lineType = static_cast<int>(COLLIDER_TYPE::LINE);
	// 線分コライダが無ければ処理を抜ける
	if (ownColliders_.count(lineType) == 0) return;
	
	// 線分コライダ情報
	ColliderLine* colliderLine =
		dynamic_cast<ColliderLine*>(ownColliders_.at(lineType));
	if (colliderLine == nullptr) return;
	// 線分の始点と終点を取得
	// 登録されている衝突物を全てチェック
	bool isHit = false;
	for (const auto& hitCol : hitColliders_)
	{
		// 落下中しか判定しない
		if (!(VDot(SchoolUtility::DIR_D, jumpPow_) > 0.9f)) continue;

		// ステージ以外は処理を飛ばす
		if (hitCol->GetTag() != ColliderBase::TAG::STAGE) continue;

		// 派生クラスへキャスト
		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(hitCol);

		if (colliderModel == nullptr) continue;

		// 衝突したポリゴンの法線方向に押し戻す
		if (colliderLine->PushBack(colliderModel, transform_, COLLISION_BACK_DIS, SchoolUtility::DIR_U, true, false))
			isHit = true;
	}

	if (isHit)
	{
		if (isJump_)
		{
			// ジャンプ中に着地したら
			isTrgLanding_ = true;
		}
		isJump_ = false;
	}

	if (!isJump_)
	{
		// ジャンプリセット
		jumpPow_ = SchoolUtility::VECTOR_ZERO;
		// ジャンプの入力受付時間をリセット
		stepJump_ = 0.0f;
	}
}

void CharacterBase::CollisionCapsule(void)
{
	// カプセルコライダ
	int capsuleType = static_cast<int>(COLLIDER_TYPE::CAPSULE);
	// カプセルコライダが無ければ処理を抜ける
	if (ownColliders_.count(capsuleType) == 0) return;

	// カプセルコライダ情報
	ColliderCapsule * colliderCapsule =
		dynamic_cast<ColliderCapsule*>(ownColliders_.at(capsuleType));

	if (colliderCapsule == nullptr) return;

	// 登録されている衝突物を全てチェック
	for (const auto& hitCol : hitColliders_)
	{
		// モデル以外は処理を飛ばす
		if (hitCol->GetShape() != ColliderBase::SHAPE::MODEL) continue;
		// 派生クラスへキャスト
		const ColliderModel* colliderModel =
			dynamic_cast<const ColliderModel*>(hitCol);
		if (colliderModel == nullptr) continue;

		colliderCapsule->PushBackAlongNormal(
			colliderModel, transform_, CNT_TRY_COLLISION, COLLISION_BACK_DIS, true, false);

	}
}

void CharacterBase::DrawShadow(void)
{
	if (imgShadow_ == -1) return;
	
	static constexpr float PLAYER_SHADOW_HEIGHT = 800.0f;
	static constexpr float PLAYER_SHADOW_SIZE = 30.0f;

	int i;
	MV1_COLL_RESULT_POLY_DIM HitResDim;
	MV1_COLL_RESULT_POLY* HitRes;
	VERTEX3D Vertex[3];
	VECTOR SlideVec;

	// ライティングを無効にする
	SetUseLighting(FALSE);

	// Ｚバッファを有効にする
	SetUseZBuffer3D(TRUE);

	// テクスチャアドレスモードを CLAMP にする
	SetTextureAddressMode(DX_TEXADDRESS_CLAMP);

	// 頂点データで変化が無い部分をセット（共通設定）
	Vertex[0].dif = GetColorU8(255, 255, 255, 255);
	Vertex[0].spc = GetColorU8(0, 0, 0, 0);
	Vertex[0].su = 0.0f;
	Vertex[0].sv = 0.0f;
	Vertex[1] = Vertex[0];
	Vertex[2] = Vertex[0];

	// hitColliders_ に登録されたステージ系コライダを順にチェックする
	int checkedModels = 0;
	for (const auto& hitCol : hitColliders_)
	{
		// ステージ以外は飛ばす
		if (hitCol->GetTag() != ColliderBase::TAG::STAGE) continue;

		// ColliderModel にキャストしてモデルハンドルを取得
		const ColliderModel* colliderModel = dynamic_cast<const ColliderModel*>(hitCol);
		if (colliderModel == nullptr) continue;

		int modelHandle = colliderModel->GetFollow()->modelId;
		if (modelHandle == 0) continue;

		// プレイヤー直下の地面ポリゴンを取得（カプセル判定）
		HitResDim = MV1CollCheck_Capsule(
			modelHandle, -1,
			transform_.pos,
			VAdd(transform_.pos, VGet(0.0f, -PLAYER_SHADOW_HEIGHT, 0.0f)),
			PLAYER_SHADOW_SIZE);

		// 検出ポリゴンがあれば描画
		HitRes = HitResDim.Dim;
		for (i = 0; i < HitResDim.HitNum; i++, HitRes++)
		{
			// ポリゴンの座標を地面ポリゴンの座標にする
			Vertex[0].pos = HitRes->Position[0];
			Vertex[1].pos = HitRes->Position[1];
			Vertex[2].pos = HitRes->Position[2];

			// 少し持ち上げて重ならないようにする
			SlideVec = VScale(HitRes->Normal, 0.5f);
			Vertex[0].pos = VAdd(Vertex[0].pos, SlideVec);
			Vertex[1].pos = VAdd(Vertex[1].pos, SlideVec);
			Vertex[2].pos = VAdd(Vertex[2].pos, SlideVec);

			// 不透明度設定
			Vertex[0].dif.a = 0;
			Vertex[1].dif.a = 0;
			Vertex[2].dif.a = 0;
			if (HitRes->Position[0].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[0].dif.a = static_cast<unsigned char>(128 * (1.0f - fabs(HitRes->Position[0].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT));
			if (HitRes->Position[1].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[1].dif.a = static_cast<unsigned char>(128 * (1.0f - fabs(HitRes->Position[1].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT));
			if (HitRes->Position[2].y > transform_.pos.y - PLAYER_SHADOW_HEIGHT)
				Vertex[2].dif.a = static_cast<unsigned char>(128 * (1.0f - fabs(HitRes->Position[2].y - transform_.pos.y) / PLAYER_SHADOW_HEIGHT));

			// UV はプレイヤーとの相対位置から算出
			Vertex[0].u = (HitRes->Position[0].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[0].v = (HitRes->Position[0].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[1].u = (HitRes->Position[1].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[1].v = (HitRes->Position[1].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[2].u = (HitRes->Position[2].x - transform_.pos.x) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;
			Vertex[2].v = (HitRes->Position[2].z - transform_.pos.z) / (PLAYER_SHADOW_SIZE * 2.0f) + 0.5f;

			// 影ポリゴンを描画
			DrawPolygon3D(Vertex, 1, imgShadow_, TRUE);
		}

		// ポリゴン情報の後始末
		MV1CollResultPolyDimTerminate(HitResDim);

		checkedModels++;
	}

	// ライティングを有効にする
	SetUseLighting(TRUE);

	// Ｚバッファを無効にする
	SetUseZBuffer3D(FALSE);
}
