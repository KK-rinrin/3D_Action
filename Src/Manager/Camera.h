#pragma once
#include <DxLib.h>
#include "../Common/Quaternion.h"
#include "../Object/Actor/ActorBase.h"
class Transform;
class ColliderSphere;
class InputBinder;

class Camera : public ActorBase
{

public:
	// 衝突判定種別
	enum class COLLIDER_TYPE
	{
		SPHERE,
		MAX
	};

	// カメラの初期座標
	static constexpr VECTOR DERFAULT_POS = { 0.0f, 200.0f, -500.0f };
	
	// カメラの初期角度
	static constexpr VECTOR DERFAULT_ANGLES = { 
		0.0f, 0.0f, 0.0f
	};

	// カメラの回転量
	const float ROT_POW_DEG = 2.0f;
	const float ROT_POW_RAD = ROT_POW_DEG * DX_PI_F / 180.0f;

	// カメラの移動スピード
	static constexpr float SPEED = 20.0f;

	// カメラのクリップ範囲
	static constexpr float VIEW_NEAR = 1.0f;
	static constexpr float VIEW_FAR = 20000.0f;

	// 追従位置からカメラ位置までの相対座標
	static constexpr VECTOR FOLLOW_CAMERA_LOCAL_POS = { 0.0f, 50.0f, -400.0f };

	// 追従位置から注視点までの相対座標
	static constexpr VECTOR FOLLOW_TARGET_LOCAL_POS = { 0.0f, 0.0f, 500.0f };

	// カメラのX回転上限度角
	static constexpr float LIMIT_X_UP_RAD = 40.0f * (DX_PI_F / 180.0f);
	static constexpr float LIMIT_X_DW_RAD = 10.0f * (DX_PI_F / 180.0f);
	
	// カメラモード
	enum class MODE
	{
		NONE,
		FIXED_POINT,
		FREE,
		FOLLOW,
	};

	// コンストラクタ
	Camera(InputBinder* inputBinder);

	// デストラクタ
	~Camera(void) override;

	// 入力判定クラスを取得
	void SetInputBinder(InputBinder* inputBinder) { inputBinder_ = inputBinder; }

	// 更新
	void Update(void) override;

	// 描画前のカメラ設定
	void SetBeforeDraw(void);

	// デバッグ用描画
	void DrawDebug(void);

	// 解放
	void Release(void);

	// 座標の取得
	const VECTOR& GetPos(void) const;

	// 角度の取得
	const VECTOR& GetAngles(void) const;
	const Quaternion& GetQuaRot(void) const;
	
	// X回転を抜いたY軸のみのカメラ角度
	const Quaternion& GetQuaRotY(void) const;
	
	// 注視点の取得
	const VECTOR& GetTargetPos(void) const;

	// カメラの前方方向
	VECTOR GetForward(void) const;

	// カメラモードの変更
	void ChangeMode(MODE mode);

	// 追従対象の設定
	void SetFollow(const Transform* follow);

private:
	// 衝突時の押し戻し試行回数
	static constexpr int CNT_TRY_COLLISION_CAMERA = 30;
	// 衝突時の押し戻し量
	static constexpr float COLLISION_BACK_DIS = 2.0f;
	// 衝突判定用球体半径
	static constexpr float COL_CAPSULE_SPHERE = 50.0f;

	// モデルの透明度
	static constexpr float COL_MODEL_ALPHA = 0.5f;

	// カメラの補間移動率
	static constexpr float LERP_RATE_MOVE = 0.1f;

	// カメラの更新前位置
	VECTOR prePos_;

	// カメラが追従対象とするTransform
	const Transform* followTransform_;

	// 入力バインダー
	InputBinder* inputBinder_;

	// カメラモード
	MODE mode_;

	// カメラ角度(rad)
	VECTOR angles_;

	// カメラ角度(Y軸のみ)
	Quaternion rotY_;

	// 注視点
	VECTOR targetPos_;
	
	// カメラを初期位置に戻す
	void SetDefault(void);

	// 追従対象との位置同期を取る
	void SyncFollow(void);

	// カメラ操作
	void ProcessRot(bool isLimit);
	void ProcessMove(void);

	// モード別更新ステップ
	void SetBeforeDrawFixedPoint(void);
	void SetBeforeDrawFree(void);
	void SetBeforeDrawFollow(void);

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

	// 衝突判定
	void Collision(void);
};
