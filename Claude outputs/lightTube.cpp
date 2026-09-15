#include "main.h"
#include "lightTube.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "audio.h"
#include "soundManager.h"
#include <cstdlib>

namespace
{
	const float kFlickerMinInterval = 0.04f;
	const float kFlickerMaxInterval = 0.35f;

	// STEP22/23: minimum real time between crackle SE plays -- see the
	// STEP21/22 note in Update() below. STEP22's 0.5s still let it fire
	// constantly (the flicker can flip every 0.04s), so widened further.
	const float kFlickerSEMinGap = 3.0f;

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
	if (m_FlickerSE) { SoundManager::Unregister(m_FlickerSE); m_FlickerSE->Uninit(); }
}

// STEP21: lazy so a non-flickering tube (most of them -- see Map.cpp's
// SpawnLightFixture isLit/flicker split) never loads the crackle mp3 at
// all. Guarded so calling SetFlicker(true) more than once on the same tube
// doesn't reload/leak a second Audio component.
void LightTube::SetFlicker(bool flicker)
{
	m_FlickerActive = flicker;

	if (flicker && !m_FlickerSE)
	{
		m_FlickerSE = AddComponent<Audio>();
		m_FlickerSE->Load("audio\\SE\\Fluorescent_Light-Noise01-1(Crackle).mp3");
		// STEP22: "電気の音がうるさい". STEP24: 直接SetVolume()せず、0.4fを
		// 基準音量としてSoundManagerに登録(設定画面のSE音量スライダーが
		// この上に掛かる)。
		SoundManager::RegisterSe(m_FlickerSE, 0.4f);
	}
}

void LightTube::Update()
{
	GameObject::Update();

	// STEP22: cooldown ticks regardless of m_FlickerActive so it's always
	// correct to check below, same reasoning as m_FlashTimer in horror.cpp.
	if (m_FlickerSECooldown > 0.0f)
	{
		m_FlickerSECooldown -= 1.0f / 60.0f;
		if (m_FlickerSECooldown < 0.0f) m_FlickerSECooldown = 0.0f;
	}

	if (m_FlickerActive)
	{
		m_FlickerTimer += 1.0f / 60.0f;

		if (m_FlickerTimer >= m_FlickerNextEventTime)
		{
			m_FlickerTimer = 0.0f;
			m_FlickerNextEventTime = RandomRange(kFlickerMinInterval, kFlickerMaxInterval);
			m_FlickerOn = !m_FlickerOn;

			// STEP21/22: one crackle per on/off flip, not per frame -- but no
			// more than one every kFlickerSEMinGap seconds either, since flips
			// can come faster than that (see kFlickerMinInterval) and stacking
			// Play() calls that fast just sounded like noise.
			if (m_FlickerSE && m_FlickerSECooldown <= 0.0f)
			{
				m_FlickerSE->Play(false);
				m_FlickerSECooldown = kFlickerSEMinGap;
			}
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