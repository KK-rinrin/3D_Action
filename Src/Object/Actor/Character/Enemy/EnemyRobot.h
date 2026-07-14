#pragma once
#include <vector>
#include <DxLib.h>
#include "EnemyBase.h"
class UISurprise;
class ShotBase;

class EnemyRobot : public EnemyBase
{
public:
	// 状態
	enum class STATE
	{
		NONE,
		THINK,
		IDLE,
		PATROL,
		SURPRISE,
		ALERT,
		CHASE,
		ATTACK_KICK,
		ATTACK_SHOOT,
		ESCAPE,
		DEAD,
		KNOCKBACK,
		END
	};

	// アニメーション種別
	enum class ANIM_TYPE
	{
		DANCE = 0,
		DIE = 1,
		HIT = 3,
		IDLE = 5,
		KICK = 9,
		RUN = 12,
		SHOOT = 13,
		WALK = 15,
	};

	// コンストラクタ
	EnemyRobot(const EnemyBase::EnemyData& data);
	// デストラクタ
	~EnemyRobot(void) override;

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
private:
	// モデルの大きさ
	static constexpr float SCALE = 0.5f;
	// モデルの回転調整
	static constexpr VECTOR DEFAULT_LOCAL_ROT =
	{ 0.0f, 180.0f * DX_PI_F / 180.0f, 0.0f };
	// 衝突判定用線分開始
	static constexpr VECTOR COL_LINE_START_LOCAL_POS =
	{ 0.0f, 80.0f, 0.0f };
	// 衝突判定用線分終了
	static constexpr VECTOR COL_LINE_END_LOCAL_POS =
	{ 0.0f, -10.0f, 0.0f };
	// 衝突判定(移動範囲)用線分開始
	static constexpr VECTOR COL_LINE_START_LOCAL_MOVE_POS =
	{ 0.0f, 80.0f, 400.0f };
	// 衝突判定(移動範囲)用線分終了
	static constexpr VECTOR COL_LINE_END_LOCAL_MOVE_POS =
	{ 0.0f, -10.0f, 400.0f };
	// 衝突判定用カプセル上部球体
	static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS =
	{ 0.0f, 40.0f, 80.0f };
	// 衝突判定用カプセル下部球体
	static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS =
	{ 0.0f, 40.0f, -40.0f };
	// 衝突判定用カプセル球体半径
	static constexpr float COL_CAPSULE_RADIUS = 30.0f;

	// 視野判定用円錐頂点
	static constexpr VECTOR COL_CONE_APEX_LOCAL_POS = { 0.0f, 0.0f, 0.0f };
	// 視野判定用円錐軸方向
	static constexpr VECTOR COL_CONE_AXIS_LOCAL_DIR = { 0.0f, 0.0f, 1.0f };

	// 巡回時の視野モデルの大きさ
	static constexpr VECTOR VIEW_RANGE_SCL = { 8.0f, 4.0f, 2.0f };
	// 巡回時の視野モデルの傾きX
	static constexpr float VIEW_RANGE_ROT_X = 26.0f * DX_PI_F / 180.0f;
	static constexpr float VIEW_RANGE_LOCAL_ROT_X = 90.0f * DX_PI_F / 180.0f;
	// 巡回時の視野モデル位置同期用フレーム番号
	static constexpr int VIEW_RANGE_SYNC_FRAME_IDX = 6;
	// 巡回時の視野の広さ
	static constexpr float VIEW_RANGE_PATROL = 600.0f;
	// 巡回時の視野角(左右半角)
	static constexpr float VIEW_ANGLE_PATROL = 30.0f;
	// 視野判定用円錐の縦倍率
	static constexpr float VIEW_RANGE_VERTICAL_SCALE = 0.25f;

	// 視野位置の調整
	static constexpr VECTOR VIEW_POS_FIX = {0.0f,10.0f,0.0f};

	enum class VIEW_COLOR
	{
		BLUE,
		YELLOW,
		RED
	};
	std::map<VIEW_COLOR, COLOR_F> viewColors_;

	// 待機時間
	static constexpr float IDLE_TIME = 3.0f;
	// 追加ランダム待機時間
	static constexpr int IDLE_TIME_RAND = 3;

	// 警戒時の振り向き角度
	static constexpr float TURNING_ANGLE = 60.0f;
	// 警戒時の振り向き往復回数
	static constexpr int TURNING_ROUND_TRIP_MAX = 2;
	// 警戒時の振り向きスピード（1.0f＝1往復1秒）
	static constexpr float TURNING_SPEED = 0.2f;

	// おどろきUI相対高さ
	static constexpr float UI_LOCAL_HEIGHT = 120.0f;
	// おどろきUI跳ねる高さ
	static constexpr float UI_BOUNCE_HEIGHT = 80.0f;
	// おどろきUIの大きさ
	static constexpr float UI_SIZE = 96.0f;
	
	// おどろき停止時間
	static constexpr float SURPRISE_TIME = 1.5f;

	// 追跡スピード
	static constexpr float CHASE_SPEED = 8.0f;
	// 最低追跡時間
	static constexpr float CHASE_TIME = 13.0f;
	// 追跡延長時間
	static constexpr float CHASE_CONTINUE = 1.5f;


	// ターゲットが入ったら近接攻撃を実行する範囲
	static constexpr float ATTACK_RANGE = 180.0f;
	// 近接攻撃クールタイム
	static constexpr float ATTACK_COOLTIME = 0.8f;
	// 近接攻撃中に近づく速度
	static constexpr float KICK_APPROACH_SPEED = 2.0f;
	// 近接攻撃中に近づく移動量の補間率(高いほどキビキビ、下げるとぬるっと)
	static constexpr float KICK_APPROACH_LERP_RATE = 0.2f;
	// 移動するにも近すぎ判定
	static constexpr float TOO_NEAR_RANGE = 20.0f;

	// 遠距離攻撃間隔の最大時間(これ÷100＋2秒)
	static constexpr int RAND_TIME = 300;

	// 状態
	STATE state_;

	// 更新ステップ
	float step_;

	// 巡回ルート座標
	std::vector<VECTOR> wayPoints_;
	// 現在、移動先としている巡回先ルートのインデックス
	int activeWayPointIndex_;
	// 現在、移動先としている巡回先座標
	VECTOR nextWayPoint_;

	// idle回数
	int idleStep_ = 0;
	// patrol回数
	int patrolStep_ = 0;

	// 視野範囲用トランスフォーム
	Transform viewRangeTransform_;

	// 警戒角度クォータニオン
	Quaternion turningRight_;
	Quaternion turningLeft_;
	// 警戒時の振り向き位相
	float turningPhase_;

	// おどろきUI
	UISurprise* uiSurprise_;

	// 状態遷移
	void ChangeState(STATE state);		// 状態変更
	void ChangeStateNone(void);			// 初期状態
	void ChangeStateThink(void);		// 思考状態
	void ChangeStateIdle(void);			// 待機状態
	void ChangeStatePatrol(void);		// 巡回状態
	void ChangeStateSurprise(void);		// おどろき状態
	void ChangeStateAlert(void);		// 警戒状態
	void ChangeStateChase(void);		// 追跡状態
	void ChangeStateAttackKick(void);	// 近接攻撃状態
	void ChangeStateAttackShoot(void);	// 遠距離攻撃状態
	void ChangeStateEscape(void);		
	void ChangeStateDead(void);			// 死亡状態
	void ChangeStateKnockBack(void);	
	void ChangeStateEnd(void);			

	// 更新系
	void UpdateNone(void);			// 初期状態
	void UpdateThink(void);			// 思考状態
	void UpdateIdle(void);			// 待機状態
	void UpdatePatrol(void);		// 巡回状態
	void UpdateSurprise(void);		// おどろき状態
	void UpdateAlert(void);			// 警戒状態
	void UpdateChase(void);			// 追跡状態
	void UpdateAttackKick(void);	// 近接攻撃状態
	void UpdateAttackShoot(void);	// 遠距離攻撃状態
	void UpdateEscape(void);
	void UpdateDead(void);			// 死亡状態
	void UpdateKnockBack(void);
	void UpdateEnd(void);

	// 視野内のプレイヤーを検知する
	bool DetectPlayer(void);

	// 巡回ルートの移動方向を設定する
	void SetMoveDirPatrol(void);

	// XZ平面上のプレイヤー方向取得
	VECTOR GetLookPlayerXZ(void);

	// 視野色変更
	void ChangeViewRangeColor(COLOR_F& color) const;

	// 自身とターゲットの距離を測定
	float GetDistanceToP();

private:
	// --- 弾関連 ---
	// 弾発射位置同期用フレーム番号
	static constexpr int SHOT_SYNC_FRAME_IDX = 1;

	static constexpr float SHOT_SPEED = 1200.0f;	// 発射速度（距離/秒）

	static constexpr float SHOT_RADIUS = 20.0f;	// 半径

	// 遠距離攻撃専用ステップ
	float shootStep_;

	ShotBase* shots_;
};