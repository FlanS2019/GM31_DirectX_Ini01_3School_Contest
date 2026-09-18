#include "main.h"
#include "interact.h"
#include "interactable.h"
#include "manager.h"
#include "camera.h"
#include "Input.h"
#include "hud.h"
#include "player.h"
#include "horror.h" // STEP15: DrawScreenEffects() -- see the note above Hud::Begin() below
#include "pauseMenu.h" // STEP24
#include "settingsScreen.h" // STEP24
#include <cstring>

namespace
{
	// SPEC doesn't give a distance; matches Door's original hand-tuned range.
	const float kMaxInteractDistance = 4.0f;

	// STEP14: hotbar redesign -- was a fixed checklist of just the 3 supply
	// items, always shown with [x]/[ ] checkboxes even before pickup. Per
	// spec ("ホットバーてきなインタラクトが欲しい...使ったら消えるように
	// したい") a slot used to only exist while Player::HasKey(id) was true.
	//
	// STEP20: "取得した時の枠は常に表示しておいてほしい" -- the slot itself
	// (background frame) is now always drawn for every id below, at a fixed
	// position, so the row doesn't jump around as items are gained/spent.
	// Only the ICON inside is conditional on HasKey() now -- see the loop in
	// Draw() -- so an empty slot still shows its frame with nothing in it,
	// and Door::Interact()/ItemBox::Interact() calling Player::RemoveKey(id)
	// still clears just the icon, not the frame. Covers every id Player's
	// key-mask is used for -- see Map.cpp's SetKeyId()/SetItemId()/
	// SetFinalKeyId() call sites.
	struct InventoryEntry { int KeyId; const char* Name; };
	const InventoryEntry kInventoryItems[] =
	{
		{ 0, "錆びた鍵" },
		{ 1, "古い写真" },
		{ 2, "色褪せた手紙" },
		{ 3, "金属部品" },
		{ 4, "組み上げた鍵" },
	};
	const int kInventoryItemCount = 5;

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

	// STEP16: "マイクラのホットバーみたいに" request -- each held id gets a
	// square slot with a small vector icon instead of a text label. There's
	// no image/texture loading pipeline in this engine to draw a real
	// thumbnail with, so this is a flat procedural icon per item id built
	// out of Hud's new DrawFilledRect/Ellipse/Polygon primitives (hud.h).
	void DrawItemIcon(int keyId, float cx, float cy, float size)
	{
		switch (keyId)
		{
		case 0: // 錆びた鍵
		case 4: // 組み上げた鍵
		{
			bool isFinal = (keyId == 4);
			float r = isFinal ? 0.85f : 0.55f;
			float g = isFinal ? 0.65f : 0.42f;
			float bch = isFinal ? 0.15f : 0.15f;

			float bowR = size * 0.20f;
			float bowCy = cy - size * 0.22f;
			float shaftW = size * 0.12f;
			float shaftTop = bowCy + bowR * 0.7f;
			float shaftBottom = cy + size * 0.30f;

			Hud::DrawRingEllipse(cx, bowCy, bowR, bowR, size * 0.07f, r, g, bch);
			Hud::DrawFilledRect(cx - shaftW * 0.5f, shaftTop, shaftW, shaftBottom - shaftTop, r, g, bch);
			Hud::DrawFilledRect(cx, shaftBottom - size * 0.20f, size * 0.14f, size * 0.06f, r, g, bch);
			Hud::DrawFilledRect(cx, shaftBottom - size * 0.08f, size * 0.11f, size * 0.05f, r, g, bch);
			break;
		}
		case 1: // 古い写真 -- dark cardboard border + sepia inset, same two-tone idea as Item_Photo.obj
		{
			Hud::DrawFilledRect(cx - size * 0.28f, cy - size * 0.32f, size * 0.56f, size * 0.64f, 0.18f, 0.14f, 0.10f);
			Hud::DrawFilledRect(cx - size * 0.20f, cy - size * 0.24f, size * 0.40f, size * 0.42f, 0.62f, 0.52f, 0.38f);
			break;
		}
		case 2: // 色褪せた手紙 -- a folded card with a visible crease line, echoing Item_Letter.obj's tent fold
		{
			Hud::DrawFilledRect(cx - size * 0.30f, cy - size * 0.22f, size * 0.60f, size * 0.44f, 0.58f, 0.54f, 0.38f);
			float xs[3] = { cx - size * 0.30f, cx, cx + size * 0.30f };
			float ys[3] = { cy - size * 0.22f, cy + size * 0.06f, cy - size * 0.22f };
			Hud::DrawFilledPolygon(xs, ys, 3, 0.42f, 0.39f, 0.27f);
			break;
		}
		case 3: // 金属部品 -- a hex nut silhouette, matching Item_MetalPart.obj's shape
		{
			const int N = 6;
			float xs[N], ys[N];
			for (int i = 0; i < N; i++)
			{
				float angle = XM_PIDIV2 + i * (XM_2PI / N);
				xs[i] = cx + cosf(angle) * size * 0.30f;
				ys[i] = cy + sinf(angle) * size * 0.30f;
			}
			Hud::DrawFilledPolygon(xs, ys, N, 0.38f, 0.38f, 0.40f);
			Hud::DrawFilledEllipse(cx, cy, size * 0.12f, size * 0.12f, 0.10f, 0.10f, 0.11f);
			break;
		}
		default:
			break;
		}
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

	// STEP37: 「クリックしても反応しない」対策 -- Eキーに加えて左クリックでも
	// 調べる/開ける操作ができるようにする。IsMouseCaptureEnabled()で
	// ゲームプレイ中(視点操作でカーソルが中央固定されている間)だけに絞る --
	// カーソルが自由なとき(一時停止/設定画面が別に開いている等)にまで
	// クリックが誤爆しないようにするため。
	bool clickInteract = Input::IsMouseCaptureEnabled() && Input::GetMouseLeftTrigger();

	if (m_Target && (Input::GetKeyTrigger('E') || clickInteract))
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

	// STEP14: Hud::Begin()/End() still always run every frame (unchanged
	// from STEP12) so the hotbar row below can appear/disappear on its own
	// the moment the player picks something up or spends it, with no
	// separate "does anything need to be drawn" check up here.

	// STEP20: "テキストと枠組みが重なっている" -- the hotbar slot row is now
	// always on screen (see the STEP20 note further down), so the warning
	// text needs a fixed position that always clears it, not a magic number
	// that happened to work back when the row only showed up sometimes.
	// Both this and the hotbar block below now read from the same two
	// constants so they can't drift out of sync with each other again.
	const float kSlotSize = 96.0f; // STEP34: 1.5x
	const float kSlotY = SCREEN_HEIGHT - kSlotSize - 30.0f; // STEP34: 1.5x
	const float kWarningY = kSlotY - 45.0f; // STEP34: 1.5x // clearly above the slot row, not overlapping it

	Hud::Begin();

	// STEP15: vignette + jump-scare flash first, so they sit underneath the
	// hotbar/prompt/warning text drawn below instead of covering it. Direct2D
	// only allows one BeginDraw()/EndDraw() pair per frame, so this can't be
	// Horror's own Draw() (Manager would call it outside this Begin()/End()
	// bracket) -- it's a plain method Horror exposes for exactly this call.
	{
		Horror* horror = Manager::GetGameObject<Horror>();
		if (horror) horror->DrawScreenEffects();
	}

	{
		// STEP16: "マイクラのホットバーみたいに" request -- square slots with
		// icons (DrawItemIcon() above) instead of text badges.
		//
		// STEP20: "取得した時の枠は常に表示しておいてほしい" -- was: only draw a
		// slot at all for ids HasKey() was true for, so the row would shrink/
		// grow and slots would shuffle position as items were gained or spent.
		// Now every id in kInventoryItems always gets a slot at a fixed
		// position; only the icon inside is conditional on HasKey(), so an
		// empty slot just shows its frame with nothing drawn in it yet.
		// kSlotSize/kSlotY are declared up above (before Hud::Begin()) now --
		// see the comment there -- so the warning text below can line up
		// against the same slot row instead of guessing its own position.
		Player* player = Manager::GetGameObject<Player>();
		if (player)
		{
			const float slotGap = 12.0f; // STEP34: 1.5x
			const float totalWidth = kSlotSize * kInventoryItemCount + slotGap * (kInventoryItemCount - 1);
			const float startX = (SCREEN_WIDTH - totalWidth) * 0.5f;

			for (int i = 0; i < kInventoryItemCount; i++)
			{
				float x = startX + i * (kSlotSize + slotGap);
				Hud::DrawPanel(x, kSlotY, kSlotSize, kSlotSize);
				if (player->HasKey(kInventoryItems[i].KeyId))
					DrawItemIcon(kInventoryItems[i].KeyId, x + kSlotSize * 0.5f, kSlotY + kSlotSize * 0.5f, kSlotSize);
			}
		}
	}

	if (hasPrompt)
	{
		Hud::DrawText(GetPromptText(), promptX, promptY, 33.0f, true); // STEP34: 1.5x
	}

	if (hasWarning)
	{
		Hud::DrawText(s_WarningText, SCREEN_WIDTH * 0.5f, kWarningY, 36.0f, true); // STEP34: 1.5x
	}

	// STEP24: 一時停止/設定画面は常に一番上に描く(モーダルなオーバーレイ
	// なので、ホットバーや警告テキストより後、Hud::End()の直前)。
	// PauseMenu::DrawUI()は設定画面が開いている間は自分から何も描かない
	// (pauseMenu.cpp参照)ので、両方呼んでも二重には描画されない。
	{
		PauseMenu* pauseMenu = Manager::GetGameObject<PauseMenu>();
		if (pauseMenu) pauseMenu->DrawUI();

		SettingsScreen* settings = Manager::GetGameObject<SettingsScreen>();
		if (settings) settings->DrawUI();
	}

	// STEP37: シーン遷移の黒フェード。PauseMenu/SettingsScreenより後、
	// Hud::End()の直前 -- 遷移中は本当に最前面が真っ黒になってほしいので。
	Hud::DrawFullScreenTint(0.0f, 0.0f, 0.0f, Manager::GetFadeAlpha());

	Hud::End();
}
