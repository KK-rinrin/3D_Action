#include "ShadowRenderer.h"
#include "../../Manager/SceneManager.h"
#include "../../Object/Common/Transform.h"
#include "../../Object/Collider/ColliderBase.h"
#include "../../Object/Collider/ColliderModel.h"

ShadowRenderer::ShadowRenderer()
	:
	shadowMapHandle_(-1),
	lightDir_(SceneManager::LIGHT_DIRECTION)
{
	if (USE_REAL_SHADOW)
	{
		shadowMapHandle_ = MakeShadowMap(SHADOW_MAP_WIDTH, SHADOW_MAP_HEIGHT);
	}
}

ShadowRenderer::~ShadowRenderer()
{
	Release();
}

void ShadowRenderer::Release(void)
{
	if (shadowMapHandle_ != -1)
	{
		DeleteShadowMap(shadowMapHandle_);
		shadowMapHandle_ = -1;
	}
}

bool ShadowRenderer::IsEnabled(void) const
{
	return USE_REAL_SHADOW && shadowMapHandle_ != -1;
}

void ShadowRenderer::BuildShadowMap(
	const Transform& center,
	const std::vector<const ColliderBase*>& hitColliders,
	const std::vector<const Transform*>& casterTransforms)
{
	if (!IsEnabled()) return;

	SetShadowMapLightDirection(shadowMapHandle_, lightDir_);

	VECTOR areaMin = VAdd(center.pos, VGet(-SHADOW_DRAW_AREA_HALF, -SHADOW_DRAW_AREA_HEIGHT, -SHADOW_DRAW_AREA_HALF));
	VECTOR areaMax = VAdd(center.pos, VGet(SHADOW_DRAW_AREA_HALF, SHADOW_DRAW_AREA_HEIGHT, SHADOW_DRAW_AREA_HALF));
	SetShadowMapDrawArea(shadowMapHandle_, areaMin, areaMax);

	ShadowMap_DrawSetup(shadowMapHandle_);
	DrawModelsForShadow(hitColliders, casterTransforms, SHADOW_MAX_MODEL_DRAW);
	ShadowMap_DrawEnd();
}

void ShadowRenderer::BeginApplyShadowMap(void) const
{
	if (!IsEnabled()) return;
	SetUseShadowMap(TRUE, shadowMapHandle_);
}

void ShadowRenderer::EndApplyShadowMap(void) const
{
	if (!IsEnabled()) return;
	SetUseShadowMap(FALSE, -1);
}

void ShadowRenderer::DrawModelsForShadow(
	const std::vector<const ColliderBase*>& hitColliders,
	const std::vector<const Transform*>& casterTransforms,
	int maxDraw)
{
	int drawn = 0;
	for (const auto& hitCol : hitColliders)
	{
		if (drawn >= maxDraw) break;
		if (hitCol->GetTag() != ColliderBase::TAG::STAGE) continue;
		const ColliderModel* cm = dynamic_cast<const ColliderModel*>(hitCol);
		if (cm == nullptr) continue;
		int modelHandle = cm->GetFollow()->modelId;
		if (modelHandle == -1 || modelHandle == 0) continue;
		MV1DrawModel(modelHandle);
		++drawn;
	}

	for (const auto& transform : casterTransforms)
	{
		if (drawn >= maxDraw) break;
		if (transform == nullptr) continue;
		if (transform->modelId == -1 || transform->modelId == 0) continue;
		MV1DrawModel(transform->modelId);
		++drawn;
	}
}