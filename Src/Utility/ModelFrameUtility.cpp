#include "../Utility/SchoolUtility.h"
#include "../Object/Common/Transform.h"
#include "ModelFrameUtility.h"

void ModelFrameUtility::GetFrameWorldMatrix(
	int modelId, int frameIdx, VECTOR& scl, MATRIX& matRot, VECTOR& pos)
{
	// 対象フレームのローカル座標からワールド座標に変換する行列を得る
	// ( 大きさ、回転、位置 )
	auto mat = MV1GetFrameLocalWorldMatrix(modelId, frameIdx);

	// 拡大縮小成分
	scl = MGetSize(mat);

	// 回転成分＋拡大縮小成分
	matRot = MGetRotElem(mat);

	// 回転成分のみにする
	auto revScl = VGet(1.0f / scl.x, 1.0f / scl.y, 1.0f / scl.z);
	matRot = MMult(matRot, MGetScale(revScl));

	// 移動成分
	pos = MGetTranslateElem(mat);
}

void ModelFrameUtility::SetFrameWorldMatrix(
	const Transform& follow, int followFrameIdx,
	Transform& target, VECTOR localPos, VECTOR localRot)
{
	// 対象フレームのローカル座標からワールド座標に変換する行列を得る
	// ( 大きさ、回転、位置 )
	MATRIX worldMatMix =
		MV1GetFrameLocalWorldMatrix(
			follow.modelId,
			followFrameIdx);

	// 拡大率を取得
	VECTOR scl = MGetSize(worldMatMix);

	// 回転成分を取得
	MATRIX frameRot = MGetRotElem(worldMatMix);

	// スケール除去
	VECTOR revScl =
		VGet(1.0f / scl.x, 1.0f / scl.y, 1.0f / scl.z);

	// 大きさを１に戻す
	frameRot =
		MMult(
			frameRot,
			MGetScale(revScl));

	// フレーム位置取得
	VECTOR framePos =
		VGet(
			worldMatMix.m[3][0],
			worldMatMix.m[3][1],
			worldMatMix.m[3][2]);

	// ローカル回転をQuaternion化
	Quaternion localQ =
		Quaternion::Euler(localRot);

	// ローカル回転を行列化
	MATRIX localRotMat =
		localQ.ToMatrix();

	// ボーン回転 + ローカル回転をターゲットに反映
	target.matRot =
		MMult(
			localRotMat,
			frameRot);

	// ローカル座標をボーン基準で回転
	VECTOR worldOffset =
		VTransform(localPos, frameRot);

	// 最終座標
	target.pos =
		VAdd(
			framePos,
			worldOffset);
}