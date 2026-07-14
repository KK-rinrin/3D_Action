#pragma once
#include <DxLib.h>
#include "EnemyBase.h"

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
		KNOCKBACK,
		DEAD,
		END
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

	// 移動範囲内へ戻る速度
	static constexpr float MOVE_IN_RANGE_SPEED = 4.0f;
	// 移動範囲の境界から内側へ取る余裕
	static constexpr float MOVE_IN_RANGE_MARGIN = 150.0f;
	// 移動範囲内へ戻る目標への到着判定距離
	static constexpr float MOVE_IN_RANGE_ARRIVE_RADIUS = 30.0f;

	// 死亡後のアニメーション終了後に小さくなるまでの時間
	static constexpr float DEAD_END_STEP = 1.0f;

	// 状態
	STATE state_;

	// 更新ステップ
	float step_;

	// 移動範囲内へ戻る目標座標
	VECTOR moveInRangeTargetPos_;

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStateThink(void);
	void ChangeStateIdle(void);
	void ChangeStateWander(void);
	void ChangeStateMoveInRange(void);
	void ChangeStateKnockBack(void);
	void ChangeStateDead(void);
	void ChangeStateEnd(void);

	// 更新系
	void UpdateNone(void);
	void UpdateThink(void);
	void UpdateIdle(void);
	void UpdateWander(void);
	void UpdateMoveInRange(void);
	//void UpdateKnockBack(void);
	void UpdateDead(void);
	void UpdateEnd(void);

};