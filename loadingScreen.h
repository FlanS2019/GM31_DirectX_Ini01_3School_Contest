#pragma once
#include "gameObject.h"
#include <vector>
#include <functional>

class LoadingScreen : public GameObject
{
private:
	struct Step
	{
		std::function<void()> Action;
		const char* Label;
	};

	std::vector<Step> m_Steps;
	size_t m_StepIndex = 0;

public:
	void Init() override;
	void Update() override;
	void Draw() override;
};
