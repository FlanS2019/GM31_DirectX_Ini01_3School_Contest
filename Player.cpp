#include "main.h"
#include "player.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "Input.h"
#include "camera.h"
#include "manager.h"
#include "bullet.h"
#include <cmath>
#include <algorithm>
#include "tree.h"
#include "box.h"
#include "audio.h"
#include "shadow.h"

void Player::Init()
{
	m_Layer = 2;

	m_Position = { 0, 0, 0 }; // start position

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\player.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");

	m_JumpSE = AddComponent<Audio>();
	m_JumpSE->Load("audio\\wan.mp3");

	m_Shadow = Manager::AddGameObject<Shadow>();
	m_Shadow->SetScale({ 5.0f, 5.0f, 5.0f });
}

void Player::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
	if (m_JumpSE) { m_JumpSE->Uninit(); }
}

void Player::Update()
{
	// fixed-step dt (fine for a school-contest build; swap for a real delta time later)
	float dt = 1.0f / 60.0f;

	const float accel = 1.0f;
	const float maxSpeed = 5.0f;
	const float friction = 15.0f;
	const float gravity = 60.0f;
	const float jumpImpulse = 25.0f;

	// --- Sprint (Shift) ---
	const float sprintMultiplier = Input::GetKeyPress(VK_SHIFT) ? 2.0f : 1.0f;
	const float currentAccel = accel * sprintMultiplier;
	const float currentMaxSpeed = maxSpeed * sprintMultiplier;

	// --- First-person: body yaw always matches the camera's look direction ---
	Camera* camera = Manager::GetGameObject<Camera>();
	float camYaw = camera ? camera->GetYaw() : 0.0f;
	m_Rotation.y = camYaw;

	Vector3 camForward(sinf(camYaw), 0.0f, cosf(camYaw));
	Vector3 camRight(cosf(camYaw), 0.0f, -sinf(camYaw));

	// --- Movement input (relative to view direction) ---
	bool moving = false;
	float inputX = 0.0f;
	float inputZ = 0.0f;

	if (Input::GetKeyPress('D')) { inputX += 1.0f; moving = true; } // right
	if (Input::GetKeyPress('A')) { inputX -= 1.0f; moving = true; } // left
	if (Input::GetKeyPress('W')) { inputZ += 1.0f; moving = true; } // forward
	if (Input::GetKeyPress('S')) { inputZ -= 1.0f; moving = true; } // back

	Vector3 moveDir = camForward * inputZ + camRight * inputX;
	float moveLen = moveDir.length();
	if (moveLen > 0.0001f)
	{
		moveDir.x /= moveLen;
		moveDir.z /= moveLen;
	}

	// --- Ground check ---
	const float groundEpsilon = 0.001f;
	bool grounded = (m_Position.y <= groundEpsilon);
	bool oldGround = m_Grounded;
	m_Grounded = false;

	if (grounded)
	{
		m_Position.y = 0.0f;
		if (Input::GetKeyTrigger(VK_SPACE))
		{
			m_Velocity.y = jumpImpulse;

			m_Scale.x = 1.0f;
			m_Scale.y = 1.5f;
			m_Scale.z = 1.0f;
		}
		else
		{
			m_Scale.x = 1.0f;
			m_Scale.y = 1.0f;
			m_Scale.z = 1.0f;
			m_JumpSE->Play();
		}

		if (!oldGround && m_Grounded)
		{
			m_Scale.x = 1.0f;
			m_Scale.y = 0.5f;
			m_Scale.z = 1.0f;
		}
	}

	// gravity
	m_Velocity.y -= gravity * dt;

	// horizontal velocity magnitude (used for friction)
	Vector3 horizontalVel(m_Velocity.x, 0.0f, m_Velocity.z);
	float hSpeed = std::sqrt(horizontalVel.x * horizontalVel.x + horizontalVel.z * horizontalVel.z);

	if (grounded)
	{
		if (moving)
		{
			m_Velocity.x += moveDir.x * 15.0f * dt;
			m_Velocity.z += moveDir.z * 15.0f * dt;

			float currentSpeed = std::sqrt(m_Velocity.x * m_Velocity.x + m_Velocity.z * m_Velocity.z);
			if (currentSpeed > maxSpeed * sprintMultiplier)
			{
				float inv = maxSpeed / currentSpeed;
				m_Velocity.x *= inv;
				m_Velocity.z *= inv;
			}
		}
		else
		{
			// no input: decelerate with friction
			if (hSpeed > 0.0f)
			{
				float decel = friction * dt;
				if (decel >= hSpeed)
				{
					m_Velocity.x = 0.0f;
					m_Velocity.z = 0.0f;
				}
				else
				{
					float inv = 1.0f / hSpeed;
					m_Velocity.x -= m_Velocity.x * inv * decel;
					m_Velocity.z -= m_Velocity.z * inv * decel;
				}
			}
		}
	}

	// integrate velocity into position
	m_Position.x += m_Velocity.x * dt;
	m_Position.y += m_Velocity.y * dt;
	m_Position.z += m_Velocity.z * dt;

	// floor collision
	if (m_Position.y < 0.0f)
	{
		m_Position.y = 0.0f;
		if (m_Velocity.y < 0.0f) m_Velocity.y = 0.0f;
		m_Grounded = true;
	}

	// tree collision
	auto trees = Manager::GetGameObjects<Tree>();
	for (auto tree : trees)
	{
		Vector3 direction = tree->GetPosition() - m_Position;
		float length = direction.length();
		if (length < 1.0f)
		{
			Vector3 pushDir = direction * (1.0f - length);
			m_Position -= pushDir;
		}
	}

	// box collision
	auto boxes = Manager::GetGameObjects<Box>();
	for (auto box : boxes)
	{
		Vector3 boxPosition = box->GetPosition();
		Vector3 boxScale = box->GetScale();
		if (boxPosition.x - boxScale.x < m_Position.x && m_Position.x < boxPosition.x + boxScale.x &&
			boxPosition.y - boxScale.y < m_Position.y && m_Position.y < boxPosition.y + boxScale.y &&
			boxPosition.z - boxScale.z < m_Position.z && m_Position.z < boxPosition.z + boxScale.z)
		{
			// resolve along whichever axis has the smallest penetration
			// (simple minimum-translation-vector push-out). Needed now that
			// Box is also used for the map's walls, which have to block
			// movement on both X and Z, not just X.
			float penTop = (boxPosition.y + boxScale.y) - m_Position.y;
			float penBottom = m_Position.y - (boxPosition.y - boxScale.y);
			float penX = boxScale.x - std::fabs(m_Position.x - boxPosition.x);
			float penZ = boxScale.z - std::fabs(m_Position.z - boxPosition.z);

			float minPen = std::min(std::min(penTop, penBottom), std::min(penX, penZ));

			if (minPen == penTop)
			{
				m_Position.y = boxPosition.y + boxScale.y;
				if (m_Velocity.y < 0.0f) m_Velocity.y = 0.0f;
				m_Grounded = true;
			}
			else if (minPen == penX)
			{
				if (m_Position.x < boxPosition.x)
					m_Position.x = boxPosition.x - boxScale.x;
				else
					m_Position.x = boxPosition.x + boxScale.x;
			}
			else
			{
				if (m_Position.z < boxPosition.z)
					m_Position.z = boxPosition.z - boxScale.z;
				else
					m_Position.z = boxPosition.z + boxScale.z;
			}
		}
	}

	if (Input::GetKeyTrigger('F')) // fire (keep or remove later depending on the game design)
	{
		OutputDebugStringA("Bullet Create\n");
		Bullet* bullet = Manager::AddGameObject<Bullet>();
		bullet->SetPosition(m_Position);
		bullet->SetVelocity(GetForward() * 25.0f);
	}

	if (m_Grounded)
	{
		m_MoveAnimetion += m_Velocity.length() * dt;
		m_Scale.y += sinf(m_MoveAnimetion * 3.0f) * 0.05f;
	}

	Vector3 shadowPos = m_Position;
	shadowPos.y = 0.05f;
	m_Shadow->SetPosition(shadowPos);

	GameObject::Update();
}


void Player::Draw()
{
	// The camera sits at the player's eye height, inside this mesh, so drawing
	// the placeholder body here puts its head/nose geometry right against the
	// near plane and it fills the screen. Skip drawing the body in first person
	// until a real model (or a proper first-person arms/view-model) is in.
	// Flip this back to true once that's ready.
	const bool kDrawBody = false;
	if (!kDrawBody)
	{
		return;
	}

	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y + XM_PI, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}