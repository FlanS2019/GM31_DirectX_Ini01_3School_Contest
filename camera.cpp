#include "main.h"
#include "camera.h"
#include "renderer.h"
#include "manager.h"
#include "player.h"
#include "Input.h"

void Camera::Init()
{
	m_Yaw = 0.0f;
	m_Pitch = 0.0f;
	m_Position = { 0, 1.6f, 0 };
}

void Camera::Uninit()
{
}

void Camera::Update()
{
	// --- Mouse look ---
	const float sensitivity = 0.0025f; // radians per pixel, tune to taste
	const float pitchLimit = 1.5f;     // ~85 degrees, avoids gimbal flip

	float dx = Input::GetMouseDeltaX();
	float dy = Input::GetMouseDeltaY();

	m_Yaw += dx * sensitivity;
	m_Pitch -= dy * sensitivity; // moving mouse up -> look up

	if (m_Pitch > pitchLimit)  m_Pitch = pitchLimit;
	if (m_Pitch < -pitchLimit) m_Pitch = -pitchLimit;

	while (m_Yaw > XM_PI)  m_Yaw -= XM_2PI;
	while (m_Yaw < -XM_PI) m_Yaw += XM_2PI;

	// --- Position: locked to the player's eye height ---
	Player* player = Manager::GetGameObject<Player>();
	Vector3 playerPos = player ? player->GetPosition() : Vector3(0.0f, 0.0f, 0.0f);

	const float eyeHeight = 1.6f; // adjust to match the player collider later
	m_Position = playerPos + Vector3(0.0f, eyeHeight, 0.0f);

	// --- Look direction from yaw/pitch ---
	Vector3 forward;
	forward.x = cosf(m_Pitch) * sinf(m_Yaw);
	forward.y = sinf(m_Pitch);
	forward.z = cosf(m_Pitch) * cosf(m_Yaw);
	forward.normalize();

	m_Target = m_Position + forward;
}

void Camera::Draw()
{
	// build the projection matrix
	XMMATRIX projection = XMMatrixPerspectiveFovLH
	(1, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);

	Renderer::SetProjectionMatrix(projection);

	XMFLOAT3 up = XMFLOAT3(0, 1, 0);
	XMFLOAT3 pos = { m_Position.x, m_Position.y, m_Position.z };
	XMFLOAT3 target = { m_Target.x, m_Target.y, m_Target.z };
	m_ViewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&pos),
		XMLoadFloat3(&target), XMLoadFloat3(&up));

	Renderer::SetViewMatrix(m_ViewMatrix);
}