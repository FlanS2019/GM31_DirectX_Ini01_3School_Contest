#pragma once
#include "gameObject.h"

class GameStartText : public GameObject
{
private:
	float m_Timer = 0.0f;

	// STEP02(stage2)-fix: used to always show the stage-1 label regardless
	// of which stage actually spawned this object -- loadingScreen.cpp and
	// undergroundLoadingScreen.cpp both add a GameStartText at the end of
	// their step lists, so without a per-stage label the underground stage
	// showed "îpö–ÅAÇÕÇ∂Çﬂ" too. Defaults to stage 1's own text so a caller that
	// never calls SetLabel() (i.e. loadingScreen.cpp, if it's ever left as-
	// is) keeps behaving exactly as before.
	char m_Label[64] = "îpö–ÅAÇÕÇ∂Çﬂ";

public:
	void Init() override;
	void Update() override;
	void Draw() override;

	void SetLabel(const char* label);
};
