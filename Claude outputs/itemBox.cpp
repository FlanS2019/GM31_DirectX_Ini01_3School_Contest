#include "main.h"
#include "itemBox.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "player.h"
#include "manager.h"
#include "interact.h"
#include "horror.h" // STEP15: TriggerJumpScare() on open

void ItemBox::Init()
{
	// A squat box so it reads as furniture, not another wall segment --
	// same reasoning as Switch's pedestal scale.
	m_Scale = { 0.6f, 0.5f, 0.6f };

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\box.obj"); // placeholder art, same as everything else this pass

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

void ItemBox::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
}

void ItemBox::Update()
{
	GameObject::Update();
}

bool ItemBox::AllItemsCollected() const
{
	Player* player = Manager::GetGameObject<Player>();
	if (!player) return false;

	for (int id : m_RequiredItemIds)
	{
		if (id < 0) continue; // unset slot -- ignore
		if (!player->HasKey(id)) return false;
	}
	return true;
}

const char* ItemBox::GetInteractText()
{
	return AllItemsCollected() ? "E ŠJ‚¯‚é" : "E ’²‚×‚é";
}

void ItemBox::Interact()
{
	if (m_Opened) return;

	if (!AllItemsCollected())
	{
		OutputDebugStringA("[ItemBox] not all 3 items collected yet.\n");
		Interact::ShowWarning("‚Ü‚¾‰½‚©‘«‚è‚È‚¢‚æ‚¤‚¾...");
		return;
	}

	Player* player = Manager::GetGameObject<Player>();
	if (player)
	{
		// STEP14: hotbar "Žg‚Á‚½‚çÁ‚¦‚é" request -- the 3 supply items are
		// spent now that they've been deposited, so drop them from the
		// inventory mask the same way Door::Interact() drops a used key.
		for (int id : m_RequiredItemIds)
		{
			if (id >= 0) player->RemoveKey(id);
		}

		if (m_FinalKeyId >= 0)
			player->AddKey(m_FinalKeyId);
	}

	m_Opened = true;
	OutputDebugStringA("[ItemBox] opened -- final key obtained.\n");

	// STEP15: scripted scare the moment the box opens -- a startling SE +
	// a quick screen flash (see horror.h/.cpp). Doesn't touch item/key
	// handling above at all, just an extra one-line hook.
	Horror* horror = Manager::GetGameObject<Horror>();
	if (horror) horror->TriggerJumpScare();
}

void ItemBox::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// box.obj shares the same local convention as everywhere else it's
	// used (Box/Door/Switch) -- see switch.cpp's Draw() for the same math.
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y - m_Scale.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}
