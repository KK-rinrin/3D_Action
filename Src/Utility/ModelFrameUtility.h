#pragma once
#include <DxLib.h>
class Transform;

class ModelFrameUtility
{
public:
#pragma region ワールド系
	// 対象フレームのワールド行列を大きさ・回転・位置に分解してを取得する
	static void GetFrameWorldMatrix(
		int modelId, int frameIdx, VECTOR & scl, MATRIX & matRot, VECTOR & pos);

	// 対象フレームの位置にtargetを配置し、
	// 対象フレームの回転に加え、指定した相対座標・回転を加える
	static void SetFrameWorldMatrix(
		const Transform& follow, int followFrameIdx,
		Transform& target, VECTOR localPos, VECTOR localRot);
#pragma endregion
};