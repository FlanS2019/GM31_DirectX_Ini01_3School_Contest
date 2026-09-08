#include "main.h"
#include "interact.h"
#include "interactable.h"
#include "manager.h"
#include "camera.h"
#include "Input.h"
#include "hud.h"
#include "player.h"
#include <cstdio>
#include <cstring>

namespace
{
	// SPEC doesn't give a distance; matches Door's original hand-tuned range.
	const float kMaxInteractDistance = 4.0f;

	// STEP12: persistent inventory panel (bottom-left) -- ids match
	// Map.cpp's SetItemId()/SetDisplayName() calls for 'P'/'R'/'M'.
	struct InventoryEntry { int KeyId; const char* Name; };
	const InventoryEntry kInventoryItems[] =
	{
		{ 1, "古い写真" },
		{ 2, "診療記録" },
		{ 3, "金属部品" },
	};
	const int kInventoryItemCount = 3;

	// Standard slab (ray-vs-AABB) test. box given as world-space min/max
	// corners -- the same "GetPosition()=center, GetScale()=half-extent"
	// convention Box/Door/Map.cpp/Player.cpp's collision already use, so
	// this works unchanged for every Interactable in the game.
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

char Interact::s_WarningText[128] = {};
float Interact::s_WarningTimer = 0.0f;

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

	m_TargetObject = bestObject;
	return best;
}

void Interact::ApplyTarget(Interactable* newTarget)
{
	if (newTarget == m_Target) return;

	char buf[128];
	if (newTarget)
		sprintf_s(buf, "[Interact] target: %s\n", newTarget->GetInteractText());
	else
		sprintf_s(buf, "[Interact] target: (none)\n");
	OutputDebugStringA(buf);

	m_Target = newTarget;
}

void Interact::ShowWarning(const char* text, float seconds)
{
	if (!text) return;
	strcpy_s(s_WarningText, text);
	s_WarningTimer = seconds;
}

void Interact::Update()
{
	if (s_WarningTimer > 0.0f)
	{
		s_WarningTimer -= 1.0f / 60.0f;
		if (s_WarningTimer < 0.0f) s_WarningTimer = 0.0f;
	}

	ApplyTarget(FindTarget());

	if (m_Target && Input::GetKeyTrigger('E'))
	{
		m_Target->Interact();
		ApplyTarget(FindTarget());
	}
}

const char* Interact::GetPromptText() const
{
	return m_Target ? m_Target->GetInteractText() : "";
}

void Interact::Draw()
{
	bool hasPrompt = false;
	float promptX = 0.0f, promptY = 0.0f;

	if (m_Target && m_TargetObject)
	{
		Camera* camera = Manager::GetGameObject<Camera>();
		if (camera)
		{
			Vector3 labelPos = m_TargetObject->GetPosition();
			labelPos.y += m_TargetObject->GetScale().y + 0.35f;

			if (WorldToScreen(labelPos, camera->GetViewMatrix(), camera->GetProjectionMatrix(), promptX, promptY)
				&& promptX >= 0.0f && promptX <= SCREEN_WIDTH
				&& promptY >= 0.0f && promptY <= SCREEN_HEIGHT)
			{
				hasPrompt = true;
			}
		}
	}

	bool hasWarning = s_WarningTimer > 0.0f;

	// STEP12: the inventory panel below is now always drawn (SPEC asked for a
	// persistent frame -- "枠組み" -- so this early-return, which used to skip
	// Hud::Begin()/End() entirely whenever there was no prompt/warning, is gone.

	Hud::Begin();

	{
		const float headerHeight = 30.0f;
		const float lineHeight = 26.0f;
		const float panelWidth = 220.0f;
		const float panelHeight = headerHeight + kInventoryItemCount * lineHeight + 10.0f;
		const float panelX = 20.0f;
		const float panelY = SCREEN_HEIGHT - panelHeight - 20.0f;

		Hud::DrawPanel(panelX, panelY, panelWidth, panelHeight);
		Hud::DrawText("アイテム", panelX + 12.0f, panelY + 8.0f, 18.0f, false);

		Player* player = Manager::GetGameObject<Player>();
		for (int i = 0; i < kInventoryItemCount; i++)
		{
			bool has = player && player->HasKey(kInventoryItems[i].KeyId);
			char line[128];
			sprintf_s(line, "%s %s", has ? "[x]" : "[ ]", kInventoryItems[i].Name);
			Hud::DrawText(line, panelX + 12.0f, panelY + headerHeight + i * lineHeight, 16.0f, false);
		}
	}

	if (hasPrompt)
	{
		Hud::DrawText(GetPromptText(), promptX, promptY, 22.0f, true);
	}

	if (hasWarning)
	{
		Hud::DrawText(s_WarningText, SCREEN_WIDTH * 0.5f, SCREEN_HEIGHT - 90.0f, 24.0f, true);
	}

	Hud::End();
}