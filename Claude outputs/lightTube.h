#pragma once

#include "gameObject.h"

class Audio;

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

	// STEP21: "電気の消えたりついたりする音" -- only the tubes that actually
	// flicker need this, so it's loaded lazily from SetFlicker(true) itself
	// (see lightTube.cpp) instead of every LightTube's Init(), since most
	// fixtures in Map.cpp are steady/unlit and would never play it anyway.
	Audio* m_FlickerSE = nullptr;

	// STEP22: "電気の音がうるさい" -- the flicker itself can flip every
	// 0.04s (see kFlickerMinInterval), far faster than a crackle SE should
	// retrigger. This counts down independently of the flicker timer and
	// blocks Play() until it clears -- see lightTube.cpp.
	float m_FlickerSECooldown = 0.0f;

public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;

	void SetFlicker(bool flicker); // STEP21: now also lazy-loads m_FlickerSE -- see lightTube.cpp
	void SetLightRange(float range) { m_LightRange = range; }
};