#include "main.h"
#include "lightCase.h"
#include "renderer.h"
#include "modelRenderer.h"

void LightCase::Init()
{
	// See lightCase.h's comment -- a starting guess, not a measurement.
	m_Scale = { 0.1f, 0.1f, 0.1f };

	ModelRenderer* modelRenderer = AddComponent<ModelRenderer>();
	modelRenderer->Load("model\\Case_Obj\\Case_Obj\\Fluorecent_Light_Case_LP.obj");

	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");
}

void LightCase::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
}

void LightCase::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	// Same reasoning as LightTube::Draw() -- no base-pivot shift, m_Position
	// used directly.
	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}
