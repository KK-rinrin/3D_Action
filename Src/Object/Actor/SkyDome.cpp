#include "SkyDome.h"
#include "../../Manager/ResourceManager.h"
#include "../../Utility/SchoolUtility.h"

SkyDome::SkyDome()
{
}

SkyDome::~SkyDome()
{
}

void SkyDome::Update()
{
	Quaternion rot = Quaternion::Euler(TURN);
	transform_.quaRot = rot.Mult(transform_.quaRot);
	transform_.Update();
}

void SkyDome::Draw()
{
	if (transform_.modelId == -1) return;
	SetUseLighting(FALSE);
	MV1DrawModel(transform_.modelId);
	SetUseLighting(TRUE);
}

void SkyDome::Release()
{
}

void SkyDome::InitLoad(void)
{
	transform_.SetModel(resMng_.Load(ResourceManager::SRC::SKY_DOME).handleId_);
}

void SkyDome::InitTransform(void)
{
	transform_.scl = SCALES;
	transform_.quaRot = Quaternion::Identity();
	transform_.quaRotLocal = Quaternion::Euler(ROT);
	transform_.pos = INIT_POS;
}

void SkyDome::InitCollider(void)
{
}

void SkyDome::InitAnimation(void)
{
}

void SkyDome::InitPost(void)
{
	// Zバッファ無効化（突き抜け対策）
	MV1SetUseZBuffer(transform_.modelId, false);
	MV1SetWriteZBuffer(transform_.modelId, false);
}
