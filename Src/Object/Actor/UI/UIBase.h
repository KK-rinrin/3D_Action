#pragma once
#include "../ActorBase.h"
class UIBase : public ActorBase
{
public:
	// コンストラクタ
	UIBase(void);
	// デストラクタ
	virtual ~UIBase(void) override;

	// 有効・無効判定
	bool IsActive(void) const;

	// 有効・無効設定
	void SetActive(bool active);

	// 更新ステップ
	float GetStep(void) const;

protected:
	// 画像のハンドルID
	int img_;

	// 有効・無効
	bool active_;

	// 更新ステップ
	float step_;

};