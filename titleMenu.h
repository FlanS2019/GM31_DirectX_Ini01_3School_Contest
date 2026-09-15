#pragma once

#include "gameObject.h"

class TitleMenu : public GameObject
{
public:
	enum class State
	{
		Root,
		ConfirmQuit,
	};

private:
	State m_State = State::Root;
	int m_Selected = 0;

public:
	void Init() override { m_State = State::Root; m_Selected = 0; m_Layer = 10; }
	void Uninit() override {}
	void Update() override;
	void Draw() override;
};
