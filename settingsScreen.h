#pragma once

#include "gameObject.h"

class SettingsScreen : public GameObject
{
private:
	bool m_Open = false;
	int m_Selected = 0;

public:
	void Init() override {}
	void Uninit() override {}
	void Update() override;
	void Draw() override {} 

	bool UpdatesWhilePaused() const override { return true; }

	void Open() { m_Open = true; m_Selected = 0; }
	void Close() { m_Open = false; }
	bool IsOpen() const { return m_Open; }

	void DrawUI();
};
