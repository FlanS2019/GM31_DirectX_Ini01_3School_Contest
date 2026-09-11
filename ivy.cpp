#include "main.h"
#include "ivy.h"
#include "renderer.h"
#include "modelRenderer.h"

void Ivy::Init()
{
	// See ivy.h's comment -- native units read as centimeters (3ds Max
	// default), so 0.01 brings it to game-world meters.
	m_Scale = { 0.01f, 0.01f, 0.01f };

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\Ivy.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

void Ivy::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
}

void Ivy::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	XMMATRIX world, scale, flip, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	flip = XMMatrixRotationRollPitchYaw(XM_PI, 0.0f, 0.0f);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	world = scale * flip * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}