#include <DxLib.h>
#include "../../Utility/SchoolUtility.h"
#include "Transform.h"

Transform::Transform(void)
	:
	modelId(-1),
	scl(SchoolUtility::VECTOR_ONE),
	rot(SchoolUtility::VECTOR_ZERO),
	pos(SchoolUtility::VECTOR_ZERO),
	localPos(SchoolUtility::VECTOR_ZERO),
	matScl(MGetIdent()),
	matRot(MGetIdent()),
	matPos(MGetIdent()),
	quaRot(Quaternion()),
	quaRotLocal(Quaternion())

{
}

Transform::~Transform(void)
{
}

void Transform::Update(void)
{

	// 大きさ
	matScl = MGetScale(scl);

	// 回転
	rot = quaRot.ToEuler();
	matRot = quaRot.ToMatrix();

	// 位置
	matPos = MGetTranslate(pos);

	// 行列の合成
	MATRIX mat = MGetIdent();
	mat = MMult(mat, matScl);
	Quaternion q = quaRot.Mult(quaRotLocal);
	mat = MMult(mat, q.ToMatrix());
	mat = MMult(mat, matPos);

	// 行列をモデルに判定
	if (modelId != -1)
	{
		MV1SetMatrix(modelId, mat);
	}

}

void Transform::Release(void)
{
}

void Transform::SetModel(int model)
{
	modelId = model;
}

VECTOR Transform::GetForward(void) const
{
	return GetDir(SchoolUtility::DIR_F);
}

VECTOR Transform::GetBack(void) const
{
	return GetDir(SchoolUtility::DIR_B);
}

VECTOR Transform::GetRight(void) const
{
	return GetDir(SchoolUtility::DIR_R);
}

VECTOR Transform::GetLeft(void) const
{
	return GetDir(SchoolUtility::DIR_L);
}

VECTOR Transform::GetUp(void) const
{
	return GetDir(SchoolUtility::DIR_U);
}

VECTOR Transform::GetDown(void) const
{
	return GetDir(SchoolUtility::DIR_D);
}

VECTOR Transform::GetDir(const VECTOR& dir) const
{
	return quaRot.PosAxis(dir);
}
