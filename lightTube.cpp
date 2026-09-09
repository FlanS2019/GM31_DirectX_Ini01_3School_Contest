#include "main.h"
#include "lightTube.h"
#include "renderer.h"
#include "modelRenderer.h"
#include <cstdlib>

namespace
{
	const float kFlickerMinInterval = 0.04f;
	const float kFlickerMaxInterval = 0.35f;

	float RandomRange(float lo, float hi)
	{
		return lo + (hi - lo) * (float(rand()) / float(RAND_MAX));
	}
}

void LightTube::Init()
{
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

void LightTube::Update()
{
	GameObject::Update();

	if (m_FlickerActive)
	{
		m_FlickerTimer += 1.0f / 60.0f;

		if (m_FlickerTimer >= m_FlickerNextEventTime)
		{
			m_FlickerTimer = 0.0f;
			m_FlickerNextEventTime = RandomRange(kFlickerMinInterval, kFlickerMaxInterval);
			m_FlickerOn = !m_FlickerOn;
		}
	}
	else
	{
		m_FlickerOn = true;
	}

	if (m_FlickerOn)
	{
		Renderer::AddPointLight(XMFLOAT3(m_Position.x, m_Position.y, m_Position.z), m_LightColor, m_LightRange);
	}
}

void LightTube::Draw()
{
	if (m_FlickerActive && !m_FlickerOn)
		return; // チカチカのOFF瞬間 -- 光る見た目も一緒に消す

	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);

	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	XMMATRIX world, scale, rot, trans;
	scale = XMMatrixScaling(m_Scale.x, m_Scale.y, m_Scale.z);
	rot = XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z);
	trans = XMMatrixTranslation(m_Position.x, m_Position.y, m_Position.z);
	world = scale * rot * trans;

	Renderer::SetWorldMatrix(world);

	GameObject::Draw();
}