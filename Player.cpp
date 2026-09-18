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
#include "soundManager.h"
#include "shadow.h"


void Player::Init()
{
	m_Layer = 8;

	m_Position = { 0, 0, 0 }; // start position

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\player.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");

	m_JumpSE = AddComponent<Audio>();
	m_JumpSE->Load("audio\\SE\\wan.mp3");

	// STEP21: footstep loop + shared pickup one-shot -- see Player.h's
	// comments on m_WalkSE/m_PickupSE for why pickup SE lives here instead
	// of on Key/Item themselves.
	m_WalkSE = AddComponent<Audio>();
	m_WalkSE->Load("audio\\SE\\kawagutu_arukuoto.mp3");
	// STEP23: "•à‚­‰¹‚ð‚Å‚Á‚©‚­‚µ‚Ä‚Ù‚µ‚¢" -- STEP22's 0.45 (a reduction)
	// turned out to be too quiet to hear at all, not too loud; boosted past
	// the default 1.0. STEP24: no longer a direct SetVolume() -- 1.6f is now
	// registered with SoundManager as this SE's "base volume", with the
	// settings screen's SE-volume slider multiplying on top of it (see
	// SoundManager::RegisterSe()).
	SoundManager::RegisterSe(m_WalkSE, 1.6f);

	m_PickupSE = AddComponent<Audio>();
	m_PickupSE->Load("audio\\SE\\sei_ge_shinbun_toru01.mp3");
	SoundManager::RegisterSe(m_PickupSE, 1.0f); // STEP24

	m_Shadow = Manager::AddGameObject<Shadow>();
	m_Shadow->SetScale({ 5.0f, 5.0f, 5.0f });
}

void Player::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
	if (m_JumpSE) { m_JumpSE->Uninit(); }
	if (m_WalkSE) { SoundManager::Unregister(m_WalkSE); m_WalkSE->Uninit(); }
	if (m_PickupSE) { SoundManager::Unregister(m_PickupSE); m_PickupSE->Uninit(); }
}

// STEP21: called by Key::Update()/Item::Interact() on a successful pickup --
// see Player.h's m_PickupSE comment for why the sound plays from here
// instead of from the (about-to-be-destroyed) Key/Item object itself.
void Player::PlayPickupSE()
{
	if (m_PickupSE) m_PickupSE->Play(false);
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

	// STEP21: footstep SE loop -- only while actually moving on the ground
	// (no phantom footsteps mid-air/mid-jump). Toggled only on the frame
	// the state changes, same "Play(true) once / Stop() once" pattern
	// Horror's heartbeat and LightTube's flicker SE both already use --
	// calling Play(true) every frame would restart the loop constantly
	// instead of actually looping it.
	{
		bool walking = moving && grounded;
		if (walking && !m_WalkPlaying)
		{
			if (m_WalkSE) m_WalkSE->Play(true);
			m_WalkPlaying = true;
		}
		else if (!walking && m_WalkPlaying)
		{
			if (m_WalkSE) m_WalkSE->Stop();
			m_WalkPlaying = false;
		}
	}

	if (grounded)
	{
		//m_Position.y = 0.0f;
		//if (Input::GetKeyTrigger(VK_SPACE))
		//{
		//	m_Velocity.y = jumpImpulse;

		//	m_Scale.x = 1.0f;
		//	m_Scale.y = 1.5f;
		//	m_Scale.z = 1.0f;
		//}
		//else
		//{
		//	m_Scale.x = 1.0f;
		//	m_Scale.y = 1.0f;
		//	m_Scale.z = 1.0f;

		//}

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

	//if (!oldGround && m_Grounded)
	//{
	//	m_JumpSE->Play();
	//}

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

	auto boxes = Manager::GetGameObjects<Box>();
	const int kCollisionPasses = 4;
	Vector3 prePush = m_Position;

	for (int pass = 0; pass < kCollisionPasses; pass++)
	{
		Box* bestBox = nullptr;
		Vector3 bestBoxPos{}, bestBoxScale{};
		float bestPen = 0.0f;
		char bestAxis = 0; // 'X' or 'Z' -- no 'T' (top) case, see above

		for (auto box : boxes)
		{
			if (!box->IsBlocking()) continue; // e.g. a Door that's (fully) open

			Vector3 boxPosition = box->GetPosition();
			Vector3 boxScale = box->GetScale();

			const float skin = 0.05f;
			if (boxPosition.x - boxScale.x - skin < m_Position.x && m_Position.x < boxPosition.x + boxScale.x + skin &&
				boxPosition.y - boxScale.y - skin < m_Position.y && m_Position.y < boxPosition.y + boxScale.y + skin &&
				boxPosition.z - boxScale.z - skin < m_Position.z && m_Position.z < boxPosition.z + boxScale.z + skin)
			{
				float penX = boxScale.x - std::fabs(m_Position.x - boxPosition.x);
				float penZ = boxScale.z - std::fabs(m_Position.z - boxPosition.z);

				float minPen = std::min(penX, penZ);

				if (bestBox == nullptr || minPen < bestPen)
				{
					bestBox = box;
					bestBoxPos = boxPosition;
					bestBoxScale = boxScale;
					bestPen = minPen;
					bestAxis = (minPen == penX) ? 'X' : 'Z';
				}
			}
		}

		if (bestBox == nullptr)
		{
			break; // nothing overlapping this pass -- fully resolved
		}

		const float pushClearance = 0.3f;

		if (bestAxis == 'X')
		{
			if (m_Position.x < bestBoxPos.x)
				m_Position.x = bestBoxPos.x - bestBoxScale.x - pushClearance;
			else
				m_Position.x = bestBoxPos.x + bestBoxScale.x + pushClearance;
		}
		else
		{
			if (m_Position.z < bestBoxPos.z)
				m_Position.z = bestBoxPos.z - bestBoxScale.z - pushClearance;
			else
				m_Position.z = bestBoxPos.z + bestBoxScale.z + pushClearance;
		}
	}

	{
		Vector3 pushDelta = m_Position - prePush;
		float pushDist = pushDelta.length();
		const float maxSanePush = 3.0f;
		if (pushDist > maxSanePush)
		{
			m_Position = prePush;
			m_Velocity.x = 0.0f;
			m_Velocity.z = 0.0f;
		}
	}

	//if (Input::GetKeyTrigger('M')) // fire (keep or remove later depending on the game design)
	//{
	//	DebugLog("Bullet Create\n");
	//	Bullet* bullet = Manager::AddGameObject<Bullet>();
	//	bullet->SetPosition(m_Position);
	//	bullet->SetVelocity(GetForward() * 25.0f);
	//}

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