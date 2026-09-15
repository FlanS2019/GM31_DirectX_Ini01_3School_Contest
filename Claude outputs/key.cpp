#include "main.h"
#include "key.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "player.h"
#include "manager.h"

namespace
{
	const float kPickupRange = 1.2f;
}

void Key::Init()
{
	// Small and off the ground so it doesn't read as another wall block;
	// Map.cpp can still override both after AddGameObject<Key>().
	m_Scale = { 0.35f, 0.35f, 0.35f };
	m_Position.y = 1.0f;

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\Key_Pickup.obj"); // was model\key.obj (a plain box) -- see key.h

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

void Key::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
}

void Key::Update()
{
	const float dt = 1.0f / 60.0f;
	m_Rotation.y += 2.0f * dt; // slow spin so it reads as "pick me up", not a prop

	Player* player = Manager::GetGameObject<Player>();
	if (player)
	{
		// STEP17: pickup range bug -- this used to be a straight 3D
		// distance, but m_Position.y is 1.0 (the key floats at hand
		// height) while Player::GetPosition() is the player's floor-level
		// root, not the camera/eye. That ~1.0 unit of "free" vertical
		// distance ate most of kPickupRange's budget, so the player had to
		// be within roughly sqrt(1.2^2 - 1.0^2) =~ 0.66m horizontally --
		// much tighter than it looked, since the camera (offset up by
		// Camera::Update()'s eyeHeight) reads the key as very close well
		// before the player's root actually is. Comparing horizontal (X/Z)
		// distance only fixes this the same way "walk up to it" pickups
		// usually ignore height in other games.
		Vector3 direction = player->GetPosition() - m_Position;
		direction.y = 0.0f;
		if (direction.length() < kPickupRange)
		{
			player->AddKey(m_KeyId);
			SetDestroy(true);
		}
	}

	GameObject::Update();
}

void Key::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// Key_Pickup.obj / key.obj / box.obj all share the same local convention
	// (X/Z centered, base at local Y=0) so GetPosition() reads as this
	// object's center the same way it does everywhere else -- see
	// box.cpp's Draw() and Key_Pickup.obj's own header comment.
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y - m_Scale.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}
