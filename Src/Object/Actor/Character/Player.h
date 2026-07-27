#pragma once
#include "CharacterBase.h"
#include <functional>
#include <map>

class AnimationController;
class WeaponBase;
class WeaponBlade;
class InputBinder;

class Player : public CharacterBase
{
public:
	// 状態
	enum class STATE
	{
		NONE,
		PLAY,
		ATTACK,
	};

	enum class ANIM_TYPE
	{
		IDLE,
		RUN,
		FAST_RUN,
		JUMP,
		JUMP_LANDING,
		SPINNING_SLASH,
		VERTICAL_SLASH,
		HORIZONTAL_SLASH,
	};

public:
	Player(InputBinder* inputBinder);
	~Player();

	void Draw() override;
	// プレイヤーUIを描画
	void DrawUI(void) const;
	void Release() override;

	// 武器を取得
	WeaponBase* GetWeapon(void);

	// ダメージを受ける
	bool Damage(int damage);
	// HPを回復
	void Heal(int heal);
	// 最大HPを増加
	void IncreaseMaxHp(int amount);

	// 現在HPを取得
	int GetHp(void) const { return hp_; }
	// 最大HPを取得
	int GetMaxHp(void) const { return maxHp_; }
	// 戦闘不能判定
	bool IsDead(void) const { return hp_ <= 0; }
	// ダメージ無敵中か
	bool IsDamageInvincible(void) const { return damageInvincibleStep_ > 0.0f; }

private:
	// 初期最大HP
	static constexpr int INIT_MAX_HP = 5;
	// ダメージ無敵時間
	static constexpr float DAMAGE_INVINCIBLE_TIME = 1.0f;

	// HPバー表示位置・サイズ
	static constexpr int HP_BAR_X = 30;
	static constexpr int HP_BAR_WIDTH = 300;
	static constexpr int HP_BAR_HEIGHT = 24;

	// 移動速度(通常)
	static constexpr float SPEED_MOVE = 5.0f;
	// 移動速度(ダッシュ)
	static constexpr float SPEED_DASH = 10.0f;
	// 攻撃ダッシュ移動減衰
	static constexpr float ATTACK_DASH_MOVE_DECAY = 0.3f;

	static constexpr float SCALE = 1.0f;
	static constexpr VECTOR INIT_ROT = { 0.0f, 180.0f * (DX_PI / 180.0f), 0.0f };
	static constexpr VECTOR INIT_POS = { 0.0f, 0.0f, 0.0f };

	// 衝突判定用線分開始
	static constexpr VECTOR COL_LINE_START_LOCAL_POS = { 0.0f, 80.0f, 0.0f };
	// 衝突判定用線分終了
	static constexpr VECTOR COL_LINE_END_LOCAL_POS = { 0.0f, -10.0f, 0.0f };

	// 衝突判定用線分開始(ジャンプ時)
	static constexpr VECTOR COL_LINE_JUMP_START_LOCAL_POS =
	{ 0.0f, 130.0f, 0.0f };
	// 衝突判定用線分終了(ジャンプ時)
	static constexpr VECTOR COL_LINE_JUMP_END_LOCAL_POS =
	{ 0.0f, 50.0f, 0.0f };

	// ジャンプ力
	static constexpr float POW_JUMP_INIT = 3500.0f;
	// 持続ジャンプ力
	static constexpr float POW_JUMP_KEEP = 400.0f;
	// ジャンプ受付時間
	static constexpr float TIME_JUMP_INPUT = 0.5f;

	// 衝突判定用カプセル上部球体
	static constexpr VECTOR COL_CAPSULE_TOP_LOCAL_POS =
	{ 0.0f, 110.0f, 0.0f };
	// 衝突判定用カプセル下部球体
	static constexpr VECTOR COL_CAPSULE_DOWN_LOCAL_POS =
	{ 0.0f, 30.0f, 0.0f };
	// 衝突判定用カプセル球体半径
	static constexpr float COL_CAPSULE_RADIUS = 20.0f;

	// 衝突判定用カプセル上部球体(ジャンプ時)
	static constexpr VECTOR COL_CAPSULE_TOP_JUMP_LOCAL_POS =
	{ 0.0f, 160.0f, 0.0f };
	// 衝突判定用カプセル下部球体(ジャンプ時)
	static constexpr VECTOR COL_CAPSULE_DOWN_JUMP_LOCAL_POS =
	{ 0.0f, 80.0f, 0.0f };

#pragma region コンボ攻撃
	// 攻撃中方向補間率
	static constexpr float ATTACK_DIR_LERP_RATE = 0.7f;

	// 攻撃コンボ制御
	struct ATTACK_COMBO
	{
		// アニメーションタイプ種別
		ANIM_TYPE animType = ANIM_TYPE::IDLE;

		// コンボ受付開始ステップ
		float stepInputStart = 0.0f;
		// コンボ受付終了ステップ
		float stepInputEnd = 0.0f;

		// 衝突判定開始ステップ
		float stepCollisionStart = 0.0f;
		// 衝突判定終了ステップ
		float stepCollisionEnd = 0.0f;

		// アニメーション割り込みステップ
		float stepInterrupt = 0.0f;
		// 攻撃入力時の移動速度
		float moveSpeed = 0.0f;
		// 次のコンボに繋げるか
		bool isNextCombo = false;
		// ノックバック強度
		float knockBackPow = 0.0f;

		// コンボ受付有効ステップ
		bool IsValidCombo(float step) const
		{
			return step > stepInputStart
				&& step < stepInputEnd;
		}

		// 衝突判定有効ステップ
		bool IsValidCollsion(float step) const
		{
			return step > stepCollisionStart
				&& step < stepCollisionEnd;
		}

		// 割り込み有効ステップ
		bool IsValidInterrupt(float step) const
		{
			return step > stepInterrupt;
		}
	};
	// 攻撃コンボ状態
	enum class STATE_ATTACK_COMBO
	{
		HORIZONTAL,
		VERTICAL,
		SPINNING,
		MAX
	};

	// 攻撃コンボ状態管理
	STATE_ATTACK_COMBO stateAtkCombo_;
	// 攻撃コンボデータ
	std::map<STATE_ATTACK_COMBO, ATTACK_COMBO> atkComboData_;
#pragma endregion

#pragma region 自作旧コード（コンボ攻撃）
#if 0
	// ボタン連続攻撃判定許容ステップ
	static constexpr float BUTTON_WHILE_ATTACK_STEP = 30.0f;
	// 武器の振り方の種類
	static constexpr int ATTACK_VARIOUS = 3;

	// 攻撃中に攻撃ボタンを押した判定
	bool isTrgDownWhileAttack_ = false;
	// 連続攻撃判定
	int attackChain_ = 0;

	// 攻撃アニメーションの種類と連続攻撃の対応
	constexpr bool isAttackAnim(int t)
	{
		ANIM_TYPE animType = static_cast<ANIM_TYPE>(t);
		return animType == ANIM_TYPE::VERTICAL_SLASH ||
			animType == ANIM_TYPE::HORIZONTAL_SLASH ||
			animType == ANIM_TYPE::SPINNING_SLASH;
	}

	constexpr STATE_ATTACK_COMBO toAttackAnim(int t)
	{
		ANIM_TYPE animType = static_cast<ANIM_TYPE>(t);
		switch (animType) {
		case ANIM_TYPE::VERTICAL_SLASH:   return STATE_ATTACK_COMBO::VERTICAL;
		case ANIM_TYPE::HORIZONTAL_SLASH: return STATE_ATTACK_COMBO::HORIZONTAL;
		case ANIM_TYPE::SPINNING_SLASH:   return STATE_ATTACK_COMBO::SPINNING;
		default:               return STATE_ATTACK_COMBO::VERTICAL; // 使われないけど一応
		}
	}
	struct HitFrame
	{
		float start;
		float end;
	};
	static constexpr size_t ATTACK_ANIM_MAX = static_cast<size_t>(STATE_ATTACK_COMBO::MAX);
	static constexpr HitFrame hitFrames[ATTACK_ANIM_MAX] = {
		HitFrame{23.0f, 27.0f}, // VERTICAL_SLASH
		HitFrame{25.0f, 30.0f}, // HORIZONTAL_SLASH
		HitFrame{26.0f, 34.0f}, // SPINNING_SLASH
	};
	std::map<ANIM_TYPE, int> comboIndex = {
	{ ANIM_TYPE::HORIZONTAL_SLASH, 1 },	// 連続攻撃の1回目
	{ ANIM_TYPE::VERTICAL_SLASH, 2 },	// 連続攻撃の2回目
	{ ANIM_TYPE::SPINNING_SLASH, 0 },	// 連続攻撃の3回目
	};
	static constexpr HitFrame get(STATE_ATTACK_COMBO type) {
		return hitFrames[static_cast<size_t>(type)];
	}
#endif
#pragma endregion

	void InitLoad(void) override;
	void InitTransform(void) override;
	void InitCollider(void) override;
	void InitAnimation(void) override;
	void InitPost(void) override;

	// 更新系
	virtual void UpdateProcess(void) override;
	virtual void UpdateProcessPost(void) override;

	// 現在HP
	int hp_;
	// 最大HP
	int maxHp_;
	// ダメージ無敵残り時間
	float damageInvincibleStep_;

	// 操作
	bool isDash_ = false;
	// 入力バインダー
	InputBinder* inputBinder_;

	VECTOR GetMoveInputDir(void);
	void ProcessMove(void);
	void ProcessJump(void);
	void ProcessAttack(void);

	// 状態管理
	STATE state_;
	// 状態管理(状態遷移時初期処理)
	std::map<STATE, std::function<void(void)>> stateChanges_;
	// 状態管理(更新ステップ)
	std::function<void(void)> stateUpdate_;

	// 状態遷移
	void ChangeState(STATE state);
	void ChangeStateNone(void);
	void ChangeStatePlay(void);
	void ChangeStateAttack(void);

	// 更新系
	void UpdateNone(void);
	void UpdatePlay(void);
	void UpdateAttack(void);

	// 衝突系
	void CollisionReserve(void) override;			// 衝突準備
	void CollisionPost(void) override;				// 衝突処理後
	// 敵本体のカプセルからXZ方向へ押し戻す
	bool CollisionEnemy(void);

	// デバッグ描画
	void DrawDebug(void) override;

	WeaponBlade* weapon_;

	// 着地アニメーション終了判定
	bool isEndLandingAnim(void);
};
