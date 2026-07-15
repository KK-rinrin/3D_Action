#include "EnemyBase.h"
#include "../../../../Utility/SchoolUtility.h"
#include "../../../Collider/ColliderSphere.h"
#include "../../../Collider/ColliderCapsule.h"
#include "../../../../Manager/SceneManager.h"

EnemyBase::EnemyBase(const EnemyBase::EnemyData& data)
	:
	type_(data.type),
	hp_(data.hp),
	stateBase_(-1),
	defaultPos_(data.defaultPos),
	moveRadius_(data.radius),
	isHit_(false),
	isVisible_(true),
	targetTransform_(nullptr)
{
	// 初期座標の設定
	transform_.pos = defaultPos_;
}

EnemyBase::~EnemyBase(void)
{
}

void EnemyBase::SetTargetTransform(const Transform* targetTransform)
{
	targetTransform_ = targetTransform;
}

void EnemyBase::Draw(void)
{
	if (!isVisible_) return;

	CharacterBase::Draw();

#ifdef _DEBUG

	// 移動可能範囲のデバッグ描画
	DrawSphere3D(defaultPos_, moveRadius_, 16, 0x000099, 0x000099, false);

#endif // _DEBUG
}

void EnemyBase::InitPost(void)
{
	// 点滅用初期処理
	InitBlink();
}

void EnemyBase::CollisionPost(void)
{
	// 衝突判定(プレイヤーの武器)
	CollisionWeapon();
}

bool EnemyBase::InMovableRange(void) const
{
	bool ret = false;

	// 初期位置からのXZ平面上の距離
	VECTOR defaultPos = defaultPos_;
	VECTOR pos = transform_.pos;
	defaultPos.y = 0.0f;
	pos.y = 0.0f;
	float dis = static_cast<float>(
		SchoolUtility::SqrMagnitude(defaultPos, pos));

	// 指定距離判定
	if (dis < moveRadius_ * moveRadius_)
	{
		return true;
	}

	return ret;
}

void EnemyBase::ChangeState(int state)
{
	stateBase_ = state;

	// 各状態遷移の初期処理
	stateChanges_[stateBase_]();
}

bool EnemyBase::InSearch(float viewRange, float viewAngle)
{
	return false;
}

void EnemyBase::Hide(void)
{
	isVisible_ = false;
	movePow_ = SchoolUtility::VECTOR_ZERO;
	jumpPow_ = SchoolUtility::VECTOR_ZERO;
	transform_.scl = SchoolUtility::VECTOR_ZERO;
	transform_.Update();
	SetAllColliderValid(false);
	ClearHitCollider();
}

void EnemyBase::CollisionWeapon(void)
{
	if (isHit_ || IsInValidDamage()) return;

	for (const auto& hitCol : hitColliders_)
	{
		// 有効でなければ処理を飛ばす
		if (!hitCol->IsValid()) continue;

		// プレイヤー武器以外は処理を飛ばす
		if (hitCol->GetTag() != ColliderBase::TAG::PLAYER_WEAPON) continue;

		// 派生クラスへキャスト
		const ColliderCapsule* weaponCol =
			dynamic_cast<const ColliderCapsule*>(hitCol);

		if (weaponCol == nullptr) continue;

		ColliderCapsule* colMyCap = dynamic_cast<ColliderCapsule*>
			(ownColliders_.at(static_cast<int>(COLLIDER_TYPE::CAPSULE)));

		if (weaponCol->IsHit(colMyCap))
		{
			if (weaponCol->GetKnockBackPow() > 0.0f)
			{
				VECTOR diff = VSub(
					weaponCol->GetCenter(), colMyCap->GetCenter());

				knockBackParam_.Init(
					diff, 3000.0f * weaponCol->GetKnockBackPow());

				OnStartKnockBack();
			}
			Damage(1);
		}
	}
}

void EnemyBase::UpdateKnockBack(void)
{
	// 移動量の計算
	movePow_ = knockBackParam_.GetMovePow();

	// 力の減衰
	knockBackParam_.Decay();

	// 点滅用時間経過
	knockBackParam_.step += scnMng_.GetDeltaTime();

	// 点滅
	Blink(knockBackParam_.step);

	// ノックバック終了判定
	if (knockBackParam_.IsEnd())
	{
		OnEndKnockBack();

		// 点滅終了処理
		SetDefaultEmiColor();
	}
}

void EnemyBase::InitBlink(void)
{
	// 自己発光の強さ
	const COLOR_F EMI_POWER = { 1.0f, 0.3f, 0.3f, 0.0f };

	// マテリアルごとの初期自己発光を保存しておく
	int num = MV1GetMaterialNum(transform_.modelId);

	for (int i = 0; i < num; i++)
	{
		// 初期自己発光
		COLOR_F dif = MV1GetMaterialEmiColor(transform_.modelId, i);
		materialEmiColors_.emplace_back(dif);

		// 点滅時自己発光
		dif.r += EMI_POWER.r;
		if (dif.r > 1.0f) { dif.r = 1.0f; }

		dif.g += EMI_POWER.g;
		if (dif.g > 1.0f) { dif.g = 1.0f; }

		dif.b += EMI_POWER.b;
		if (dif.b > 1.0f) { dif.b = 1.0f; }

		materialEmiBlinkColors_.emplace_back(dif);
	}
}

void EnemyBase::Blink(float step)
{
	// 点滅間隔
	constexpr int TERM_BLINK = 5;

	// 点滅スピード
	constexpr float SPEED_BLINK = 20.0f;

	int intStep = static_cast<int>(step * SPEED_BLINK);

	// モデルの点滅処理
	int i = 0;
	if (intStep % TERM_BLINK <= TERM_BLINK / 5)
	{
		// デフォルトの自己発光色
		for (const auto& color : materialEmiColors_)
		{
			MV1SetMaterialEmiColor(transform_.modelId, i++, color);
		}
	}
	else
	{
		// 点滅時の自己発光色
		for (const auto& color : materialEmiBlinkColors_)
		{
			MV1SetMaterialEmiColor(transform_.modelId, i++, color);
		}
	}
}

void EnemyBase::SetDefaultEmiColor(void)
{
	int i = 0;
	for (const auto& color : materialEmiColors_)
	{
		MV1SetMaterialEmiColor(transform_.modelId, i++, color);
	}
}

void EnemyBase::Damage(int damage)
{
	hp_ -= damage;
	if (hp_ < 0) hp_ = 0;
}
