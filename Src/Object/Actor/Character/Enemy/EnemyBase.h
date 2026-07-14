#pragma once
#include <functional>
#include <vector>
#include <map>
#include <cmath>
#include <DxLib.h>
#include "../CharacterBase.h"

class EnemyBase : public CharacterBase
{
public:
	// 種別
	enum class TYPE
	{
		RAT,
		ROBOT,
	};

	// エネミーデータ
	struct EnemyData
	{
		int id;
		EnemyBase::TYPE type;
		int hp;
		VECTOR defaultPos;
		float radius;
		std::vector<VECTOR> wayPoints;
	};

	// ノックバックパラメータ
	struct KnockBackParam
	{
		static constexpr float DECAY_RATE = 0.08f;// 減衰率

		float weight;	// 重さ
		VECTOR dir;		// ノックバック方向
		float pow;		// ノックバック力
		float step;

		// コンストラクタ
		KnockBackParam(void)
			:
			weight(0.0f),
			dir({ 0.0f, 0.0f, 0.0f }),
			pow(0.0f),
			step(0.0f)
		{
		}

		// ノックバック初期化
		void Init(VECTOR knockBackVec, float knockBackPow)
		{
			if (weight <= 0.0f) return;

			// ノックバック力
			pow = knockBackPow / weight;

			// XZ平面のノックバック方向を求める
			VECTOR dirXZ = knockBackVec;
			dirXZ.y = 0.0f;
			dirXZ = VNorm(dirXZ);

			// 上に跳ねる角度
			float angle = 40.0f * DX_PI_F / 180.0f;

			// 上に跳ねる角度と、XZ平面方向保ったまま、
			// 最終的なノックバック方向を求める
			// ( Y成分はsinで方向変換し、XZ平面成分をcosで弱める)
			VECTOR nkockBackDir = VECTOR();
			nkockBackDir.x = dirXZ.x * cosf(angle);
			nkockBackDir.z = dirXZ.z * cosf(angle);
			nkockBackDir.y = sinf(angle);

			// 正規化(省略化だが、念のため)
			dir = VNorm(nkockBackDir);
		}

		// ノックバック力を取得
		VECTOR GetMovePow(void) const
		{
			return VScale(dir, pow);
		}

		// ノックバック力を減衰させる
		void Decay(void)
		{
			pow = pow * (1.0f - DECAY_RATE);
		}

		// ノックバック終了判定
		bool IsEnd(void) const
		{
			return pow <= 0.1f;
		}
	};

	// コンストラクタ
	EnemyBase(const EnemyBase::EnemyData& data);
	// デストラクタ
	virtual ~EnemyBase(void) override;

	// 描画
	virtual void Draw(void) override;

	// 追跡対象を設定
	void SetTargetTransform(const Transform* targetTransform);

protected:
	// 種別
	TYPE type_;
	// HP
	int hp_;

	// 状態管理
	int stateBase_;

	// 初期位置
	const VECTOR defaultPos_;

	// 移動範囲
	float moveRadius_;

	// すでに攻撃が当たったか
	bool isHit_;

	// 表示中か
	bool isVisible_;

	// 追跡対象
	const Transform* targetTransform_;

	// 状態管理(状態遷移時初期処理)
	std::map<int, std::function<void(void)>> stateChanges_;

	// 状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	// ノックバックパラメータ
	KnockBackParam knockBackParam_;

	// 自己発光
	std::vector<COLOR_F> materialEmiColors_;
	std::vector<COLOR_F> materialEmiBlinkColors_;

	// リソースロード
	void InitLoad(void) override {}
	// 大きさ、回転、座標の初期化
	void InitTransform(void) override {}
	// 衝突判定の初期化
	void InitCollider(void) override {}
	// アニメーションの初期化
	void InitAnimation(void) override {}
	// 初期化後の個別処理
	void InitPost(void) override;

	// 更新系
	virtual void UpdateProcessPost(void) override {}

	// 個別の衝突判定
	virtual void CollisionPost(void) override;

	// 移動可能範囲判定
	bool InMovableRange(void) const;

	// 状態遷移
	void ChangeState(int state);

	// 非表示化
	void Hide(void);

	// 索敵範囲にターゲットがいるか
	bool InSearch(float viewRange, float viewAngle);

	// 衝突判定(プレイヤーの武器)
	void CollisionWeapon(void);

	// ダメージ無効判定
	virtual bool IsInValidDamage(void) const = 0;

	// ノックバック開始処理
	virtual void OnStartKnockBack(void) {}

	// ノックバック更新処理
	void UpdateKnockBack(void);

	// ノックバック終了処理
	virtual void OnEndKnockBack(void) {}

	// 点滅用初期処理
	void InitBlink(void);

	// 点滅処理
	void Blink(float step);

	// 初期自己発光色に戻す
	void SetDefaultEmiColor(void);

	// ダメージ処理
	void Damage(int damage);
};