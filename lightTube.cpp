#include "main.h"
#include "lightTube.h"
#include "renderer.h"
#include "modelRenderer.h"

void LightTube::Init()
{
	// See lightTube.h's comment -- a starting guess, not a measurement.
	// Map.cpp (or wherever this gets placed) can override with SetScale()
	// after AddGameObject<LightTube>() if it looks off in-game.
	m_Scale = { 0.01f, 0.01f, 0.01f };

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\Tube_Obj\\Tube_Obj\\LightTube_LP.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

void LightTube::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
}

void LightTube::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// Unlike Box/Item's Draw(), this model's own vertex data already spans
	// both + and - on every axis (roughly centered on its own origin), so
	// m_Position is used directly here -- no "- m_Scale.y" base-pivot shift
	// like box.obj/key.obj need.
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}
