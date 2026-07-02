#include "UIBase.h"
UIBase::UIBase(void)
	:
	ActorBase(),
	img_(-1),
	active_(false),
	step_(0.0f)
{
}

UIBase::~UIBase(void)
{
}

bool UIBase::IsActive(void) const
{
	return active_;
}

void UIBase::SetActive(bool active)
{
	active_ = active;
}

float UIBase::GetStep(void) const
{
	return step_;
}