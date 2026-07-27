#pragma once
#include <functional>
#include <vector>
#include <map>
#include <cmath>
#include <DxLib.h>
#include "../CharacterBase.h"

class ColliderCapsule;

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

	// 解放
	void Release(void) override;

	// 追跡対象を設定
	void SetTargetTransform(const Transform* targetTransform);

	// プレイヤーへの攻撃ダメージを取得してリセット
	int ConsumePlayerDamage(void);

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

	// プレイヤーへ与える予約ダメージ
	int playerDamage_;

	// 攻撃専用コライダ
	std::vector<ColliderBase*> attackColliders_;

	// 前方地面追従で上れる最大の高さ
	static constexpr float GROUND_FOLLOW_MAX_STEP_HEIGHT = 30.0f;
	// 前方地面追従で下へ吸着できる最大の高さ
	static constexpr float GROUND_FOLLOW_MAX_SNAP_DOWN = 30.0f;
	// 前方地面を探す線分の上下の余裕
	static constexpr float GROUND_FOLLOW_PROBE_MARGIN = 10.0f;

	// 障害物に詰まったと判定するまでの時間
	static constexpr float OBSTACLE_STUCK_TIME = 0.25f;
	// 移動できたとみなすXZ平面上の距離
	static constexpr float OBSTACLE_MOVE_DISTANCE = 0.1f;
	// 障害物を迂回する時間
	static constexpr float OBSTACLE_AVOID_TIME = 0.6f;

	// 障害物への詰まり判定時間
	float obstacleStuckStep_;
	// 障害物の迂回時間
	float obstacleAvoidStep_;
	// 障害物の迂回方向(-1.0f:左、1.0f:右)
	float obstacleAvoidSide_;
	// このフレームに進もうとした方向
	VECTOR obstacleMoveDir_;
	// このフレームに障害物への詰まりを調べるか
	bool isObstacleMoveCheck_;
	// 詰まったときに迂回するか
	bool canObstacleAvoid_;
	// 障害物に詰まったか
	bool isObstacleStuck_;

	// 状態管理(状態遷移時初期処理)
	std::map<int, std::function<void(void)>> stateChanges_;

	// 状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	// ノックバックパラメータ
	static constexpr float DAMAGED_KNOCKBACK_TIME = 0.1f;	// ノックバック時間が短い判定
	KnockBackParam knockBackParam_;	// ノックバックのパラメータ
	float damagedStep_;	// ダメージの秒

	// 被ダメ時アニメーション種別
	int animTypeDamaged_;

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
	// 衝突判定前準備
	void CollisionReserve(void) override;

	// 個別の衝突判定
	virtual void CollisionPost(void) override;

	// 移動可能範囲判定
	bool InMovableRange(void) const;

	// 障害物を考慮した移動方向を取得
	VECTOR GetObstacleAvoidMoveDir(const VECTOR& targetDir);

	// 障害物への詰まり判定を予約
	void ReserveObstacleStuckCheck(const VECTOR& moveDir);

	// 障害物への詰まり判定を取得してリセット
	bool ConsumeObstacleStuck(void);

	// 状態遷移
	void ChangeState(int state);

	// 非表示化
	void Hide(void);

	// 索敵範囲にターゲットがいるか
	bool InSearch(float viewRange, float viewAngle);

	// 衝突判定(プレイヤーの武器)
	void CollisionWeapon(void);

	// 衝突判定(敵の攻撃とプレイヤー)
	void CollisionPlayer(void);

	// 攻撃専用コライダを追加
	void AddAttackCollider(ColliderBase* attackCollider);

	// 攻撃専用コライダの有効無効設定
	void SetAllAttackCollidersValid(bool isValid);

	// プレイヤーのカプセルコライダを取得
	const ColliderCapsule* GetPlayerCapsuleCollider(void) const;

	// プレイヤーへのダメージを予約
	void ReservePlayerDamage(int damage);

	// 障害物への詰まりと迂回を更新
	void UpdateObstacleAvoidance(void);

	// ダメージ無効判定
	virtual bool IsInValidDamage(void) const = 0;

	// ノックバック開始処理
	virtual void OnStartKnockBack(void) = 0;

	// ノックバック更新処理
	void UpdateKnockBack(void);

	// ノックバック終了処理
	virtual void OnEndKnockBack(void) = 0;

	void StartDamaged(void);

	// 被ダメージ開始処理
	virtual void OnStartDamaged(void) = 0;

	// 被ダメージ更新処理
	void UpdateDamaged(void);

	// 被ダメージ終了処理
	virtual void OnEndDamaged(void) = 0;

	// 点滅用初期処理
	void InitBlink(void);

	// 点滅処理
	void Blink(float step);

	// 初期自己発光色に戻す
	void SetDefaultEmiColor(void);

	// ダメージ処理
	void Damage(int damage);
};
