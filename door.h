#pragma once

#include "box.h"
#include "interactable.h"

class Door : public Box, public Interactable
{
public:
	enum class SlideDirection { Up, PosX, NegX, PosZ, NegZ };

private:
	int m_RequiredKeyId = -1; // -1 = no key needed, 'E' alone opens it
	bool m_Open = false;      // true once opening has been triggered
	float m_OpenT = 0.0f;     // 0 = closed, 1 = fully open
	SlideDirection m_SlideDirection = SlideDirection::Up;
	Vector3 m_BasePosition;   // position the door was placed at (closed)
	bool m_BaseCaptured = false;

public:
	void Init()override;
	void Update()override;

	bool IsBlocking() const override { return m_OpenT < 1.0f; }

	const char* GetInteractText() override;
	void Interact() override;
	bool CanInteract() override { return m_OpenT < 1.0f; } // nothing left to interact with once fully open

	void SetRequiredKey(int keyId) { m_RequiredKeyId = keyId; }

	void SetSlideDirection(SlideDirection dir) { m_SlideDirection = dir; }

	void Open() { m_Open = true; }

	bool IsOpen() const { return m_OpenT >= 1.0f; }
};