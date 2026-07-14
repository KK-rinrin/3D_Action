#pragma once
#include <DxLib.h>
#include "ColliderBase.h"

class ColliderCapsule;
class Transform;

class ColliderCone : public ColliderBase
{
public:
	// コンストラクタ
	ColliderCone(
		TAG tag, const Transform* follow,
		const VECTOR& localPosApex, const VECTOR& localAxis,
		float height, float halfAngleRight, float halfAngleUp);
	// デストラクタ
	~ColliderCone(void) override;

	// 点と円錐の当たり判定を行う
	bool IsHitPoint(const VECTOR& point) const;
	// カプセルとの当たり判定を行う
	bool IsHitCapsule(const ColliderCapsule* colliderCapsule) const;

	// 衝突時に法線方向へ押し戻した位置を取得する
	VECTOR GetPosPushBackAlongNormal(
		const MV1_COLL_RESULT_POLY& hitColPoly,
		int maxTryCnt,
		float pushDistance) const override
	{
		return {};
	}

protected:
	// デバッグ描画
	void DrawDebug(int color) override;

private:
	// デバッグ描画の分割数
	static constexpr int DRAW_DIV_NUM = 16;
	// カプセル判定時のサンプリング数
	static constexpr int CAPSULE_SAMPLE_COUNT = 32;
	// 判定時の微小値
	static constexpr float HIT_EPSILON = 0.0001f;

	// ローカル座標系での頂点位置
	VECTOR localPosApex_;
	// ローカル座標系での軸方向
	VECTOR localAxis_;
	// 円錐の高さ
	float height_;
	// 右側の半開角
	float halfAngleRight_;
	// 上側の半開角
	float halfAngleUp_;

	// 軸から右方向ベクトルを計算する
	VECTOR CalcRightAxis(const VECTOR& axis) const;
	// 点を拡張半径で判定する補助関数
	bool IsHitExpandedPoint(const VECTOR& point, float expandRadius) const;
};
