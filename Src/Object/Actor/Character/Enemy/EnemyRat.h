#pragma once
#include <DxLib.h>
#include <vector>
#include "EnemyBase.h"

class UISurprise;

class EnemyRat : public EnemyBase
{
public:

	// 状態
	enum class STATE
	{
		NONE,
		THINK,
		IDLE,
		WANDER,
		MOVE_IN_RANGE,
		SURPRISE,
		CHASE,
		ATTACK,
		ESCAPE,
		KNOCKBACK,
		DAMAGED,
		DEAD,
		END
	};

	// 攻撃データ
	struct ATTACK
	{
		// 攻撃前動作のアニメーションインデックス
		int prepareAnimIndex = -1;
		// FBX内のアニメーションインデックス
		int animIndex = -1;
		// 攻撃判定の開始再生率
		float collisionStartRate = 0.0f;
		// 攻撃判定の終了再生率
		float collisionEndRate = 0.0f;

		bool IsValidCollision(float playRate) const
		{
			return playRate >= collisionStartRate
				&& playRate <= collisionEndRate;
		}
	};

	// ATTACK内の進行段階
	enum class ATTACK_PHASE
	{
		PREPARE,
		ATTACK,
	};

	// アニメーション種別
	enum class ANIM_TYPE
	{
		DIE = 6,
		HIT = 7,
		IDLE = 8,
		WALK = 13,

	};

	// コンストラクタ
	EnemyRat(const EnemyBase::EnemyData& data);

	// デストラクタ
	~EnemyRat(void) override;

	// 描画
	void Draw(void) override;
protected:
	// リソースロード
	void InitLoad(void) override;
	// 大きさ、回転、座標の初期化
	void InitTransform(void) override;
	// 衝突判定の初期化
	void InitCollider(void) override;
	// アニメーションの初期化
	void InitAnimation(void) override;
	// 初期化後の個別処理
	void InitPost(void) override;
	// 更新系
	void UpdateProcess(void) override;
	void UpdateProcessPost(void) override;

	// ダメージ無効判定
	bool IsInValidDamage(void) const override;

	// ノックバック開始処理
	void OnStartKnockBack(void) override;

	// ノックバック終了処理
	void OnEndKnockBack(void) override;

	// 被ダメージ開始処理
	void OnStartDamaged(void) override;

	// 被ダメージ終了処理
	void OnEndDamaged(void) override;

private:
	// モデルの大きさ
	static constexpr float SCALE = 0.5f;

	// モデルのローカル回転
	static constexpr VECTOR ROT = { 0.0f, 180.0f * DX_PI_F / 180.0f, 0.0f };
	
	// 初期座標
	static constexpr VECTOR INIT_POS = { 0.0f, 100.0f, 1500.0f };

	// 衝突判定用線分開始
	static constexpr VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 80.0f, 0.0f };
	// 衝突判定用線分終了
	static constexpr VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, -10.0f, 0.0f };

	// 衝突判定用カプセル上部球体
	static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS = { 0.0f, 30.0f, 70.0f };
	// 衝突判定用カプセル下部球体
	static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS = { 0.0f, 30.0f, -40.0f };
	// 衝突判定用カプセル球体半径
	static constexpr float COL_CAPSULE_RADIUS = 30.0f;

	// ATTACK用カプセル上部球体
	static constexpr VECTOR COL_ATTACK_TOP_LOCAL_POS =
	{ 0.0f, 40.0f, 100.0f };
	// ATTACK用カプセル下部球体
	static constexpr VECTOR COL_ATTACK_DOWN_LOCAL_POS =
	{ 0.0f, 25.0f, 55.0f };
	// ATTACK用カプセル球体半径
	static constexpr float COL_ATTACK_RADIUS = 40.0f;

	// 移動範囲内へ戻る速度
	static constexpr float MOVE_IN_RANGE_SPEED = 4.0f;
	// 移動範囲の境界から内側へ取る余裕
	static constexpr float MOVE_IN_RANGE_MARGIN = 150.0f;
	// 移動範囲内へ戻る目標への到着判定距離
	static constexpr float MOVE_IN_RANGE_ARRIVE_RADIUS = 30.0f;

	// 視野距離
	static constexpr float VIEW_RANGE = 320.0f;
	// 正面を中心とした左右それぞれの視野角度
	static constexpr float VIEW_ANGLE = 120.0f;
	// 追跡速度
	static constexpr float CHASE_SPEED = 6.0f;
	// 追跡時間
	static constexpr float CHASE_TIME = 3.0f;
	// 追跡延長時間
	static constexpr float CHASE_CONTINUE = 1.5f;
	// 逃走時間
	static constexpr float ESCAPE_TIME = 3.0f;
	// 逃走速度
	static constexpr float ESCAPE_SPEED = CHASE_SPEED;

	// 死亡後のアニメーション終了後に小さくなるまでの時間
	static constexpr float DEAD_END_STEP = 1.0f;

	// 攻撃前動作アニメーションのインデックス
	static constexpr int ATTACK_PREPARE_ANIM_INDEX = 4;
	// 右前足ひっかきアニメーションのインデックス
	static constexpr int ATTACK_ANIM_INDEX = 2;
	// 攻撃を開始する距離
	static constexpr float ATTACK_RANGE = 120.0f;

	// おどろきUI相対高さ
	static constexpr float UI_LOCAL_HEIGHT = 80.0f;
	// おどろきUI跳ねる高さ
	static constexpr float UI_BOUNCE_HEIGHT = 50.0f;
	// おどろきUIの大きさ
	static constexpr float UI_SIZE = 64.0f;

	// 視野投影の角度分割数
	static constexpr int VIEW_PROJECTION_ANGLE_DIVISIONS = 16;
	// 視野投影の半径分割数
	static constexpr int VIEW_PROJECTION_RADIUS_DIVISIONS = 4;
	// 視野投影線分の上方向の長さ
	static constexpr float VIEW_PROJECTION_PROBE_UP = 200.0f;
	// 視野投影線分の下方向の長さ
	static constexpr float VIEW_PROJECTION_PROBE_DOWN = 500.0f;
	// 視野投影面を地面から浮かせる高さ
	static constexpr float VIEW_PROJECTION_Y_OFFSET = 2.0f;
	// 視野投影面の不透明度
	static constexpr int VIEW_PROJECTION_ALPHA = 80;

	// 状態
	STATE state_;

	// 更新ステップ
	float step_;

	// 移動範囲内へ戻る目標座標
	VECTOR moveInRangeTargetPos_;
	// 攻撃データ
	std::vector<ATTACK> attacks_;
	// 現在の攻撃データ番号
	int attackIndex_;
	// ATTACK内の進行段階
	ATTACK_PHASE attackPhase_;
	// 視野へ再侵入したとき追跡を開始できるか
	bool canStartChase_;
	// ATTACK終了後に残り時間からCHASEを再開するか
	bool isResumeChase_;
	// おどろきUI
	UISurprise* uiSurprise_;

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStateThink(void);
	void ChangeStateIdle(void);
	void ChangeStateWander(void);
	void ChangeStateMoveInRange(void);
	void ChangeStateSurprise(void);
	void ChangeStateChase(void);
	void ChangeStateAttack(void);
	void ChangeStateEscape(void);
	void ChangeStateKnockBack(void);
	void ChangeStateDamaged(void);
	void ChangeStateDead(void);
	void ChangeStateEnd(void);

	// 更新系
	void UpdateNone(void);
	void UpdateThink(void);
	void UpdateIdle(void);
	void UpdateWander(void);
	void UpdateMoveInRange(void);
	void UpdateSurprise(void);
	void UpdateChase(void);
	void UpdateAttack(void);
	void UpdateEscape(void);
	//void UpdateKnockBack(void);
	void UpdateDead(void);
	void UpdateEnd(void);

	// 視野範囲をステージへ投影描画
	void DrawViewRangeProjection(void) const;
	// 指定したXZ座標の投影先となる地面を取得
	bool GetViewProjectionPoint(
		const VECTOR& pos, VECTOR& projectionPos) const;

	// プレイヤーが視野距離・視野角度内にいるか
	bool IsPlayerInViewRange(
		float range, float viewHalfAngle) const;
	// プレイヤーが攻撃距離内にいるか
	bool IsPlayerInAttackRange(float range) const;

};
