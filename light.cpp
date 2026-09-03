#include "main.h"
#include "light.h"
#include "manager.h"
#include "camera.h"
#include "Input.h"
#include <cstdlib>

namespace
{
	const float kFlickerMinInterval = 0.04f;
	const float kFlickerMaxInterval = 0.35f;

	const float kFlickerDimScale = 0.12f;

	float RandomRange(float lo, float hi)
	{
		return lo + (hi - lo) * (float(rand()) / float(RAND_MAX));
	}
}

void Light::Init()
{
	Push();
}

void Light::Update()
{
	// SPEC STEP5: "FƒL[‚ÅON/OFF".
	if (Input::GetKeyTrigger('F'))
	{
		ToggleFlashlight();
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
		light.Ambient = m_Ambient; // the room around the beam stays just as dark
		light.Diffuse = m_FlashlightDiffuse;
		light.SpotParams = XMFLOAT4(m_FlashlightInnerCos, m_FlashlightOuterCos, m_FlashlightRange, 0.0f);
	}
	else
	{
		light.IsSpot = false;
		light.Direction = m_Direction;
		light.Ambient = m_Ambient;

		float scale = m_FlickerOn ? 1.0f : kFlickerDimScale;
		light.Diffuse = XMFLOAT4(m_Diffuse.x * scale, m_Diffuse.y * scale, m_Diffuse.z * scale, 1.0f);
	}

	Renderer::SetLight(light);
}

void Light::StopFlicker()
{
	m_FlickerActive = false;
	m_FlickerOn = true;
}