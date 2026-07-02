#pragma once
#include <DxLib.h>
#include <vector>

class Transform;
class ColliderBase;

/// <summary>
/// シーンで共有するリアルシャドウ管理
/// </summary>
class ShadowRenderer
{
public:
	static constexpr bool USE_REAL_SHADOW = true;

	ShadowRenderer();
	~ShadowRenderer();

	void Release(void);

	bool IsEnabled(void) const;
	void BuildShadowMap(
		const Transform& center,
		const std::vector<const ColliderBase*>& hitColliders,
		const std::vector<const Transform*>& casterTransforms);
	void BeginApplyShadowMap(void) const;
	void EndApplyShadowMap(void) const;

private:
	// シャドウマップ幅（ピクセル）
	static constexpr int SHADOW_MAP_WIDTH = 4096;
	// シャドウマップ高さ（ピクセル）
	static constexpr int SHADOW_MAP_HEIGHT = 4096;
	// シャドウマップの描画範囲（中心からの半径）
	static constexpr float SHADOW_DRAW_AREA_HALF = 1000.0f;
	// シャドウマップの上下高さ（中心からの上下幅）
	static constexpr float SHADOW_DRAW_AREA_HEIGHT = 800.0f;
	// シャドウマップを描画する際のモデルの最大数保険（安全上のループ防止）
	static constexpr int SHADOW_MAX_MODEL_DRAW = 32;

	int shadowMapHandle_;
	VECTOR lightDir_;

	void DrawModelsForShadow(
		const std::vector<const ColliderBase*>& hitColliders,
		const std::vector<const Transform*>& casterTransforms,
		int maxDraw);
};