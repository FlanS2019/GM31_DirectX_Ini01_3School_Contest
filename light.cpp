#include "main.h"
#include "light.h"
#include "manager.h"
#include "camera.h"
#include "Input.h"
#include "gameSettings.h"
#include <cstdlib>
#include <cstdio>

namespace
{
	const float kFlickerMinInterval = 0.04f;
	const float kFlickerMaxInterval = 0.35f;

	const float kFlickerDimScale = 0.12f;

	const float kFlashlightIntensityStep = 0.25f;
	const float kFlashlightIntensityMin = 0.25f;
	const float kFlashlightIntensityMax = 6.0f;

	const float kBrightnessScaleMin = 0.7f;
	const float kBrightnessScaleMax = 1.3f;

	float RandomRange(float lo, float hi)
	{
		return lo + (hi - lo) * (float(rand()) / float(RAND_MAX));
	}
}

void Light::Init()
{
	SetBrightness01(GameSettings::GetBrightness());
	Push();
}

void Light::SetBrightness01(float brightness01)
{
	if (brightness01 < 0.0f) brightness01 = 0.0f;
	if (brightness01 > 1.0f) brightness01 = 1.0f;
	m_BrightnessScale = kBrightnessScaleMin + (kBrightnessScaleMax - kBrightnessScaleMin) * brightness01;
}

void Light::Update()
{
	if (Input::GetKeyTrigger('F'))
	{
		ToggleFlashlight();
	}

	if (Input::GetKeyTrigger(VK_OEM_PLUS))
	{
		m_FlashlightIntensityScale += kFlashlightIntensityStep;
		if (m_FlashlightIntensityScale > kFlashlightIntensityMax) m_FlashlightIntensityScale = kFlashlightIntensityMax;

		char buf[64];
		sprintf_s(buf, "[Light] flashlight intensity: %.2fx\n", m_FlashlightIntensityScale);
		OutputDebugStringA(buf);
	}
	if (Input::GetKeyTrigger(VK_OEM_MINUS))
	{
		m_FlashlightIntensityScale -= kFlashlightIntensityStep;
		if (m_FlashlightIntensityScale < kFlashlightIntensityMin) m_FlashlightIntensityScale = kFlashlightIntensityMin;

		char buf[64];
		sprintf_s(buf, "[Light] flashlight intensity: %.2fx\n", m_FlashlightIntensityScale);
		OutputDebugStringA(buf);
	}

	if (m_FlickerActive)
	{
		m_FlickerTimer += 1.0f / 60.0f;

		if (m_FlickerTimer >= m_FlickerNextEventTime)
		{
			m_FlickerTimer = 0.0f;
			m_FlickerNextEventTime = RandomRange(kFlickerMinInterval, kFlickerMaxInterval);
			m_FlickerOn = !m_FlickerOn;
		}
	}
	else
	{
		m_FlickerOn = true;
	}

	if (m_FlashlightOn)
	{
		UpdateFlashlightAim();
	}

	Push();
}

void Light::UpdateFlashlightAim()
{
	Camera* camera = Manager::GetGameObject<Camera>();
	if (!camera) return;

	Vector3 eye = camera->GetPosition();
	Vector3 forward = camera->GetForward();

	Vector3 worldUp(0.0f, 1.0f, 0.0f);
	Vector3 right = Vector3::cross(worldUp, forward);
	right.normalize();

	Vector3 origin = eye + forward * 0.35f + right * 0.28f + Vector3(0.0f, -0.28f, 0.0f);

	m_SpotPosition = XMFLOAT4(origin.x, origin.y, origin.z, 1.0f);
	m_SpotDirection = XMFLOAT4(forward.x, forward.y, forward.z, 0.0f);
}

void Light::Push()
{
	LIGHT light{};
	light.Enable = m_Enable;

	if (m_FlashlightOn)
	{
		light.IsSpot = true;
		light.Position = m_SpotPosition;
		light.Direction = m_SpotDirection;
		light.Ambient = XMFLOAT4(m_Ambient.x * m_BrightnessScale, m_Ambient.y * m_BrightnessScale, m_Ambient.z * m_BrightnessScale, m_Ambient.w);
		light.Diffuse = XMFLOAT4(
			m_FlashlightDiffuse.x * m_FlashlightIntensityScale * m_BrightnessScale,
			m_FlashlightDiffuse.y * m_FlashlightIntensityScale * m_BrightnessScale,
			m_FlashlightDiffuse.z * m_FlashlightIntensityScale * m_BrightnessScale,
			1.0f);
		light.SpotParams = XMFLOAT4(m_FlashlightInnerCos, m_FlashlightOuterCos, m_FlashlightRange, 0.0f);
	}
	else
	{
		light.IsSpot = false;
		light.Direction = m_Direction;
		light.Ambient = XMFLOAT4(m_Ambient.x * m_BrightnessScale, m_Ambient.y * m_BrightnessScale, m_Ambient.z * m_BrightnessScale, m_Ambient.w);

		float scale = m_FlickerOn ? 1.0f : kFlickerDimScale;
		light.Diffuse = XMFLOAT4(m_Diffuse.x * scale * m_BrightnessScale, m_Diffuse.y * scale * m_BrightnessScale, m_Diffuse.z * scale * m_BrightnessScale, 1.0f);
	}

	Renderer::SetLight(light);
}

void Light::StopFlicker()
{
	m_FlickerActive = false;
	m_FlickerOn = true;
}