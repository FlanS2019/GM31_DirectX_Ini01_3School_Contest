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
#include "Score.h"
#include <cstdio>

namespace
{
	// increments once per Player::Update call -- lets the debug log show
	// whether two [COLLIDE] lines happened in the SAME frame or several
	// frames apart, which the old log (no frame number) couldn't tell you.
	unsigned s_DebugFrame = 0;

	// DEBUG: writes every log line to a file (in addition to
	// OutputDebugStringA) so it survives independent of when the VS Output
	// window gets copied -- that window's content is only good until the
	// next F5, so a video from one run and a log copied after starting the
	// next run don't actually match. This file is truncated fresh at the
	// start of each run (Player::Init) and every line is flushed
	// immediately, so whatever's in it after closing the game is exactly
	// that run's log, however it ends (including a crash). It lands next to
	// the other relative paths this project already uses (model\\...,
	// shader\\...), i.e. wherever the .exe's working directory is -- same
	// place, every run.
	FILE* s_DebugLogFile = nullptr;

	void DebugLog(const char* text)
	{
		OutputDebugStringA(text);
		if (s_DebugLogFile)
		{
			fputs(text, s_DebugLogFile);
			fflush(s_DebugLogFile);
		}
	}
}

void Player::Init()
{
	m_Layer = 8;

	// DEBUG: fresh log file every run -- see DebugLog() above.
	if (!s_DebugLogFile)
	{
		fopen_s(&s_DebugLogFile, "debug_log.txt", "w");
	}

	m_Position = { 0, 0, 0 }; // start position

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\player.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");

	m_JumpSE = AddComponent<Audio>();
	m_JumpSE->Load("audio\\SE\\wan.mp3");

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
	s_DebugFrame++;

	// --- DEBUG: show the live frame counter where the score HUD normally
	// goes (top-left "0 0 0 0"), so a screen recording can be lined up
	// exactly against the Output-window [COLLIDE] log by just reading the
	// number on screen. Score isn't used for anything yet, so this is safe
	// to leave in until the wall bug is confirmed fixed -- just delete this
	// block (and the SetValue call it needs in Score.h) afterward.
	{
		Score* score = Manager::GetGameObject<Score>();
		if (score)
		{
			score->SetValue(s_DebugFrame % 10000);
		}
	}

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

			// BUGFIX: this used to call m_JumpSE->Play() every single grounded
			// frame that wasn't a jump-trigger frame (i.e. constantly, while
			// just standing/walking) -- moved below into the "just landed"
			// check where a landing sound actually belongs. Unrelated to the
			// wall issue, just clearly not what was intended.
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

	// --- DEBUG: catch any single-frame position jump that isn't normal
	// walking (see the "JUMP DETECTED" check further down). Remove once the
	// wall issue is confirmed fixed.
	Vector3 debugPosBeforeMove = m_Position;

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

	// landing sound: exactly the frame the player transitions from airborne
	// to grounded (replaces the old "every idle grounded frame" call above)
	if (!oldGround && m_Grounded)
	{
		m_JumpSE->Play();
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

	// box collision (also used for the map's walls)
	//
	// Each pass, find the box the player is *least* deeply overlapping (the
	// shallowest penetration = the most recently-touched, most legitimate
	// contact) and resolve only that one, then move to the next pass and
	// re-measure from scratch. This keeps every single push bounded by one
	// real box's geometry instead of letting pushes against several boxes
	// chain off of each other's side effects within the same pass (which
	// used to be able to fling the player a large distance in one frame).
	//
	// NO top/"stand on top" case anymore: these boxes are hospital walls,
	// not platforms. Previously, a box's top surface (Y = boxPos.y +
	// boxScale.y) counted as a landing spot, same as the floor. jumpImpulse
	// (25) with gravity (60) gives a jump apex of ~5.2 units -- comfortably
	// higher than a wall's 3-unit height -- so jumping anywhere near a wall
	// could land the player ON TOP of it, which then got treated as solid
	// ground (m_Grounded = true), letting them walk around up there. From
	// up there, looking out over the tops of every 3-unit wall in the maze
	// at the open field beyond looks exactly like "all the walls just
	// vanished" -- no rendering bug or actual teleport required. Debug logs
	// confirmed this: long runs of pick=T with playerPos.y sitting at
	// exactly boxPos.y+boxScale.y while moving smoothly in X/Z, i.e.
	// walking along a wall's top edge. Walls now only ever push sideways
	// (X/Z) -- there's nothing to climb.
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

			// A wall's bottom is exactly at Y=0 (boxPosition.y - boxScale.y,
			// with WALL_HEIGHT/2 for both), and the player's grounded Y is
			// forced to exactly 0.0f every frame by the floor-collision code
			// above. "0.0 < 0.0" is always false, so without this margin the
			// Y check never overlaps while grounded.
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

		// push out a bit further than the exact boundary. The camera sits
		// at the player's XZ position, and pushing to exactly touching
		// (distance 0) leaves the wall surface closer than the 0.1 near-clip
		// plane the instant you look straight at it.
		const float pushClearance = 0.3f;

		// --- DEBUG: print exactly what this collision event did, with a
		// frame number so consecutive lines can be told apart as same-frame
		// vs different-frame.
		{
			char buf[256];
			sprintf_s(buf,
				"[COLLIDE] frame=%u pass=%d box=%p boxPos=(%.2f,%.2f,%.2f) boxScale=(%.2f,%.2f,%.2f) playerPos=(%.2f,%.2f,%.2f) pen=%.3f pick=%c\n",
				s_DebugFrame, pass, (void*)bestBox, bestBoxPos.x, bestBoxPos.y, bestBoxPos.z,
				bestBoxScale.x, bestBoxScale.y, bestBoxScale.z,
				m_Position.x, m_Position.y, m_Position.z,
				bestPen, bestAxis);
			DebugLog(buf);
		}

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

	// --- DEBUG safety net: even with the one-box-per-pass change above, if
	// collision resolution this frame still moved the player further than
	// is physically sane for one box's worth of push, something is still
	// wrong -- undo it rather than let the player fly out past a wall.
	// Worst case for a single legitimate push here is roughly one box's
	// half-extent (2.0 for this map's walls) plus pushClearance (0.3), so
	// 3.0 units of total leeway across all 4 passes is generous but still
	// far below "flew across the map."
	{
		Vector3 pushDelta = m_Position - prePush;
		float pushDist = pushDelta.length();
		const float maxSanePush = 3.0f;
		if (pushDist > maxSanePush)
		{
			char buf[256];
			sprintf_s(buf,
				"[SUSPICIOUS PUSH] frame=%u collision moved %.2f units in one frame: (%.2f,%.2f,%.2f) -> (%.2f,%.2f,%.2f) -- reverted\n",
				s_DebugFrame, pushDist, prePush.x, prePush.y, prePush.z, m_Position.x, m_Position.y, m_Position.z);
			DebugLog(buf);

			m_Position = prePush;
			m_Velocity.x = 0.0f;
			m_Velocity.z = 0.0f;
		}
	}

	// --- DEBUG: flag any single-frame move bigger than what's physically
	// possible (max speed is 5, x2 sprinting = 10 units/sec, so at 60fps a
	// legit frame moves at most ~0.17 units before collision response).
	// Anything much larger than that in one frame is a teleport, not
	// walking -- if this fires right when the walls vanish in-game, the
	// [COLLIDE] / [SUSPICIOUS PUSH] lines just above it are the ones that
	// caused it.
	{
		Vector3 debugDelta = m_Position - debugPosBeforeMove;
		float debugJump = debugDelta.length();
		if (debugJump > 1.0f)
		{
			char buf[256];
			sprintf_s(buf,
				"[JUMP DETECTED] frame=%u moved %.2f units in one frame: (%.2f,%.2f,%.2f) -> (%.2f,%.2f,%.2f)\n",
				s_DebugFrame, debugJump, debugPosBeforeMove.x, debugPosBeforeMove.y, debugPosBeforeMove.z,
				m_Position.x, m_Position.y, m_Position.z);
			DebugLog(buf);
		}
	}

	if (Input::GetKeyTrigger('F')) // fire (keep or remove later depending on the game design)
	{
		DebugLog("Bullet Create\n");
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

	// --- DEBUG: one line per frame with the final position, regardless of
	// whether any collision happened. The [COLLIDE] lines only fire when
	// touching a box, so a jump that happens while NOT touching anything
	// (e.g. mid-air, or in open floor) would otherwise leave zero trace.
	// This closes that gap -- with the on-screen frame counter, any jump
	// shows up as a big gap between consecutive lines' position, at an
	// exact, readable frame number. Cheap to leave running for our short
	// test sessions; delete once the bug's confirmed fixed.
	{
		char buf[128];
		sprintf_s(buf, "[POS] frame=%u pos=(%.2f,%.2f,%.2f)\n",
			s_DebugFrame, m_Position.x, m_Position.y, m_Position.z);
		DebugLog(buf);
	}

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