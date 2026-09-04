#include "main.h"
#include "interact.h"
#include "interactable.h"
#include "manager.h"
#include "camera.h"
#include "Input.h"
#include "hud.h"
#include <cstdio>

namespace
{
	// SPEC doesn't give a distance; matches Door's original hand-tuned range.
	const float kMaxInteractDistance = 4.0f;

	// Standard slab (ray-vs-AABB) test. box given as world-space min/max
	// corners -- the same "GetPosition()=center, GetScale()=half-extent"
	// convention Box/Door/Map.cpp/Player.cpp's collision already use, so
	// this works unchanged for every Interactable in the game. Written out
	// per-axis rather than as a loop over Vector3 as float[3] -- Vector3
	// isn't guaranteed contiguous, and this is only 3 axes anyway.
	bool RayIntersectsAABB(const Vector3& origin, const Vector3& dir,
		const Vector3& boxMin, const Vector3& boxMax, float maxDist, float& outDist)
	{
		float tMin = 0.0f;
		float tMax = maxDist;

		// X
		if (fabsf(dir.x) < 1e-6f)
		{
			if (origin.x < boxMin.x || origin.x > boxMax.x) return false;
		}
		else
		{
			float invD = 1.0f / dir.x;
			float t1 = (boxMin.x - origin.x) * invD;
			float t2 = (boxMax.x - origin.x) * invD;
			if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
			if (t1 > tMin) tMin = t1;
			if (t2 < tMax) tMax = t2;
			if (tMin > tMax) return false;
		}

		// Y
		if (fabsf(dir.y) < 1e-6f)
		{
			if (origin.y < boxMin.y || origin.y > boxMax.y) return false;
		}
		else
		{
			float invD = 1.0f / dir.y;
			float t1 = (boxMin.y - origin.y) * invD;
			float t2 = (boxMax.y - origin.y) * invD;
			if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
			if (t1 > tMin) tMin = t1;
			if (t2 < tMax) tMax = t2;
			if (tMin > tMax) return false;
		}

		// Z
		if (fabsf(dir.z) < 1e-6f)
		{
			if (origin.z < boxMin.z || origin.z > boxMax.z) return false;
		}
		else
		{
			float invD = 1.0f / dir.z;
			float t1 = (boxMin.z - origin.z) * invD;
			float t2 = (boxMax.z - origin.z) * invD;
			if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
			if (t1 > tMin) tMin = t1;
			if (t2 < tMax) tMax = t2;
			if (tMin > tMax) return false;
		}

		outDist = tMin;
		return true;
	}

	// World -> screen-pixel projection for the floating prompt label.
	// Hand-written instead of XMVector3Project because that helper wants a
	// D3D11_VIEWPORT on hand, which Interact doesn't keep around --
	// SCREEN_WIDTH/HEIGHT from main.h is all this actually needs.
	bool WorldToScreen(const Vector3& worldPos, const XMMATRIX& view, const XMMATRIX& projection,
		float& outX, float& outY)
	{
		XMVECTOR pos = XMVectorSet(worldPos.x, worldPos.y, worldPos.z, 1.0f);
		XMVECTOR clip = XMVector4Transform(pos, view * projection);

		float w = XMVectorGetW(clip);
		if (w <= 0.0001f) return false; // behind the camera

		float ndcX = XMVectorGetX(clip) / w;
		float ndcY = XMVectorGetY(clip) / w;

		outX = (ndcX * 0.5f + 0.5f) * SCREEN_WIDTH;
		outY = (1.0f - (ndcY * 0.5f + 0.5f)) * SCREEN_HEIGHT;
		return true;
	}
}

Interactable* Interact::FindTarget()
{
	Camera* camera = Manager::GetGameObject<Camera>();
	if (!camera) { m_TargetObject = nullptr; return nullptr; }

	Vector3 origin = camera->GetPosition();
	Vector3 dir = camera->GetForward();

	Interactable* best = nullptr;
	GameObject* bestObject = nullptr;
	float bestDist = kMaxInteractDistance;

	for (Interactable* candidate : Manager::GetGameObjects<Interactable>())
	{
		if (!candidate->CanInteract()) continue;

		// Every Interactable in this game is also a GameObject (see
		// interactable.h) -- cross-cast back to read its position/extent.
		GameObject* gameObject = dynamic_cast<GameObject*>(candidate);
		if (!gameObject) continue;

		Vector3 pos = gameObject->GetPosition();
		Vector3 half = gameObject->GetScale();
		Vector3 boxMin = pos - half;
		Vector3 boxMax = pos + half;

		float dist;
		if (RayIntersectsAABB(origin, dir, boxMin, boxMax, bestDist, dist))
		{
			best = candidate;
			bestObject = gameObject;
			bestDist = dist;
		}
	}

	// Kept alongside m_Target so Draw() below has a world position to
	// project, without redoing the raycast or the dynamic_cast.
	m_TargetObject = bestObject;
	return best;
}

void Interact::Update()
{
	Interactable* newTarget = FindTarget();

	if (newTarget != m_Target)
	{
		char buf[128];
		if (newTarget)
			sprintf_s(buf, "[Interact] target: %s\n", newTarget->GetInteractText());
		else
			sprintf_s(buf, "[Interact] target: (none)\n");
		OutputDebugStringA(buf);

		m_Target = newTarget;
	}

	if (m_Target && Input::GetKeyTrigger('E'))
	{
		m_Target->Interact();
	}
}

const char* Interact::GetPromptText() const
{
	return m_Target ? m_Target->GetInteractText() : "";
}

void Interact::Draw()
{
	if (!m_Target || !m_TargetObject) return;

	Camera* camera = Manager::GetGameObject<Camera>();
	if (!camera) return;

	// Float the label just above the target's box (its own half-height
	// plus a little clearance) instead of at its center, so it reads as a
	// label on the door/switch/etc. rather than text buried mid-slab --
	// this is the "ƒhƒA•t‹ß‚É" placement, as opposed to a screen-center
	// HUD prompt.
	Vector3 labelPos = m_TargetObject->GetPosition();
	labelPos.y += m_TargetObject->GetScale().y + 0.35f;

	float screenX, screenY;
	if (!WorldToScreen(labelPos, camera->GetViewMatrix(), camera->GetProjectionMatrix(), screenX, screenY))
		return;

	// Off-screen (target's box is in raycast range/angle but its label
	// point landed outside the window, e.g. near a screen edge) -- don't
	// draw a label floating off the visible area.
	if (screenX < 0.0f || screenX > SCREEN_WIDTH || screenY < 0.0f || screenY > SCREEN_HEIGHT)
		return;

	Hud::Begin();
	Hud::DrawText(GetPromptText(), screenX, screenY, 22.0f, true);
	Hud::End();
}