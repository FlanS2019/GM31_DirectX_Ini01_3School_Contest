#pragma once

#include "gameObject.h"
#include "renderer.h" // LIGHT struct

class Light : public GameObject
{
private:
	bool m_Enable = true;

	// Baseline (non-flickering, non-flashlight) mood lighting. Deliberately
	// dark -- see the class comment above.
	XMFLOAT4 m_Direction = XMFLOAT4(0.3f, -1.0f, 0.2f, 0.0f);
	XMFLOAT4 m_Ambient = XMFLOAT4(0.02f, 0.02f, 0.025f, 1.0f);
	XMFLOAT4 m_Diffuse = XMFLOAT4(0.08f, 0.08f, 0.07f, 1.0f);

	bool m_FlickerActive = false;
	bool m_FlickerOn = true;
	float m_FlickerTimer = 0.0f;
	float m_FlickerNextEventTime = 0.0f; // seconds until the next on/off flip

	// STEP5: flashlight state.
	bool m_FlashlightOn = false;
	float m_FlashlightRange = 14.0f;
	float m_FlashlightInnerCos = 0.93f; // full brightness inside this cone
	float m_FlashlightOuterCos = 0.82f; // fades to 0 by this cone's edge
	XMFLOAT4 m_FlashlightDiffuse = XMFLOAT4(1.6f, 1.55f, 1.3f, 1.0f); // slightly warm bulb color
	XMFLOAT4 m_SpotPosition{};  // computed each frame from the camera, see UpdateFlashlightAim()
	XMFLOAT4 m_SpotDirection{};

	void Push(); // uploads the current state to Renderer::SetLight()
	void UpdateFlashlightAim();

public:
	void Init() override;
	void Update() override;

	void StartFlicker() { m_FlickerActive = true; m_FlickerTimer = 0.0f; m_FlickerNextEventTime = 0.0f; }
	void StopFlicker();

	// For a "the power goes out" event later; not used yet.
	void SetEnable(bool enable) { m_Enable = enable; }

	void SetDirection(const XMFLOAT4& direction) { m_Direction = direction; }
	void SetAmbient(const XMFLOAT4& ambient) { m_Ambient = ambient; }
	void SetDiffuse(const XMFLOAT4& diffuse) { m_Diffuse = diffuse; }

	// STEP5
	void SetFlashlight(bool on) { m_FlashlightOn = on; }
	void ToggleFlashlight() { m_FlashlightOn = !m_FlashlightOn; }
	bool IsFlashlightOn() const { return m_FlashlightOn; }
};