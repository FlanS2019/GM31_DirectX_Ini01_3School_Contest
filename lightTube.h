#pragma once

#include "gameObject.h"

class LightTube : public GameObject
{
private:
	ID3D11InputLayout* m_VertexLayout = nullptr;
	ID3D11VertexShader* m_VertexShader = nullptr;
	ID3D11PixelShader* m_PixelShader = nullptr;

	// この管が部屋に与える光の色・強さと届く範囲。
	XMFLOAT3 m_LightColor = XMFLOAT3(1.0f, 0.97f, 0.85f);
	float m_LightRange = 6.0f;

	// チカチカ演出。デフォルトOFF、SetFlicker(true)で有効化。
	bool m_FlickerActive = false;
	bool m_FlickerOn = true;
	float m_FlickerTimer = 0.0f;
	float m_FlickerNextEventTime = 0.0f;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetFlicker(bool flicker) { m_FlickerActive = flicker; }
	void SetLightRange(float range) { m_LightRange = range; }
};