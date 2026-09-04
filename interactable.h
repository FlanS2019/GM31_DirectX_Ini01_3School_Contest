#pragma once

class Interactable
{
public:
	virtual ~Interactable() {}
	virtual const char* GetInteractText() = 0;
	virtual void Interact() = 0;
	virtual bool CanInteract() { return true; }
};