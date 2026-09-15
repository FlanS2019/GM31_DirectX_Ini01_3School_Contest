#pragma once

#include "gameObject.h"

class Audio;

class LightTube : public GameObject
{
private:
	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

	XMFLOAT3 m_LightColor = XMFLOAT3(1.0f, 0.97f, 0.85f);
	float m_LightRange = 6.0f;

	bool m_FlickerActive = false;
	bool m_FlickerOn = true;
	float m_FlickerTimer = 0.0f;
	float m_FlickerNextEventTime = 0.0f;

	Audio* m_FlickerSE = nullptr;

	float m_FlickerSECooldown = 0.0f;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetFlicker(bool flicker); 
	void SetLightRange(float range) { m_LightRange = range; }
};