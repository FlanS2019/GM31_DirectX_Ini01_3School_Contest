#pragma once

#include "gameObject.h"

class TitleBgm : public GameObject
{
private:
	class Audio* m_Bgm = nullptr;

public:
	void Init() override;
	void Uninit() override;
	void Update() override {}
	void Draw() override {}
};
