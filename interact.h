#pragma once

#include "gameObject.h"

class Interactable;

class Interact : public GameObject
{
private:
	Interactable* m_Target = nullptr;    // whatever's currently under the reticle, or null
	GameObject* m_TargetObject = nullptr; // m_Target's GameObject half (for its world position) -- see FindTarget()

	Interactable* FindTarget();

public:
	void Init() override { m_Layer = 10; }

	void Update() override;
	void Draw() override;

	bool HasTarget() const { return m_Target != nullptr; }
	const char* GetPromptText() const;
};