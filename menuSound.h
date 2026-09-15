#pragma once

#include "gameObject.h"

class MenuSound : public GameObject
{
private:
	class Audio* m_MoveSE = nullptr;
	class Audio* m_ConfirmSE = nullptr;

public:
	void Init() override;
	void Uninit() override;
	void Update() override {}
	void Draw() override {}

	void PlayMove();

	void PlayConfirm();
};
