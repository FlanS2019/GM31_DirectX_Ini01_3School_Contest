#pragma once

#include "box.h"
#include "interactable.h"

class ModelRenderer;
class Audio;

class Door : public Box, public Interactable
{
private:
	int m_RequiredKeyId = -1; 
	bool m_Open = false;
	float m_OpenT = 0.0f;
	bool m_IsExit = false;
	bool m_ClearTriggered = false;

	// STEP01(stage2): a second, independent "opening this finishes the
	// door and changes scene" flag, parallel to m_IsExit -- see door.cpp's
	// Update(). A given Door only ever has one of the two set.
	bool m_LeadsToUnderground = false;

	// STEP01-fix(stage2) DEBUG: how long R has been held while eligible for
	// the debug key-grant (see door.cpp's Update()). Resets to 0 the moment
	// the conditions stop holding.
	float m_DebugKeyHoldTime = 0.0f;

	bool m_SwitchOnly = false;

	Audio* m_OpenSE = nullptr;

	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

	ModelRenderer* m_FrameRenderer = nullptr;
	ModelRenderer* m_LeftLeafRenderer = nullptr;
	ModelRenderer* m_RightLeafRenderer = nullptr;

public:
	void Init()override;
	void Uninit()override;
	void Update()override;
	void Draw()override;

	bool IsBlocking() const override { return m_OpenT < 1.0f; }

	const char* GetInteractText() override;
	void Interact() override;
	bool CanInteract() override { return !m_SwitchOnly && m_OpenT < 1.0f; } 

	void SetRequiredKey(int keyId) { m_RequiredKeyId = keyId; }
	void SetSwitchOnly(bool switchOnly) { m_SwitchOnly = switchOnly; }

	void Open(); 
	bool IsOpen() const { return m_OpenT >= 1.0f; }

	void SetIsExit(bool isExit) { m_IsExit = isExit; }
	void SetLeadsToUnderground(bool leadsToUnderground) { m_LeadsToUnderground = leadsToUnderground; }
};
