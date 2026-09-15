#include "main.h"
#include "door.h"
#include "player.h"
#include "manager.h"
#include "Input.h"
#include "result.h"
#include "interact.h"
#include "renderer.h"
#include "modelRenderer.h"
#include "audio.h"
#include "soundManager.h"

namespace
{
	const float kOpenSeconds = 1.0f;
	const float kOpenAngleDeg = 100.0f;

	const float kLeftHingeX = -0.4501f;
	const float kRightHingeX = 0.4048f;

	Vector3 RotateY(const Vector3& v, float yaw)
	{
		float c = cosf(yaw), s = sinf(yaw);
		return Vector3(v.x * c + v.z * s, v.y, -v.x * s + v.z * c);
	}

	XMMATRIX BuildWorld(float yaw, const Vector3& position)
	{
		XMMATRIX rot = XMMatrixRotationRollPitchYaw(0.0f, yaw, 0.0f);
		XMMATRIX trans = XMMatrixTranslation(position.x, position.y, position.z);
		return rot * trans;
	}

	XMMATRIX BuildLeafWorld(float hingeLocalX, float baseYaw, float leafAngle, const Vector3& basePos)
	{
		XMMATRIX preTrans = XMMatrixTranslation(-hingeLocalX, 0.0f, 0.0f);
		XMMATRIX rot = XMMatrixRotationRollPitchYaw(0.0f, baseYaw + leafAngle, 0.0f);
		Vector3 hingeWorld = basePos + RotateY(Vector3(hingeLocalX, 0.0f, 0.0f), baseYaw);
		XMMATRIX postTrans = XMMatrixTranslation(hingeWorld.x, hingeWorld.y, hingeWorld.z);
		return preTrans * rot * postTrans;
	}
}

void Door::Init()
{
	Renderer::CreateVertexShader(&m_VertexShader, &m_VertexLayout, "shader\\unlitTextureVS.cso");
	Renderer::CreatePixelShader(&m_PixelShader, "shader\\unlitTexturePS.cso");

	m_FrameRenderer = AddComponent<ModelRenderer>();
	m_FrameRenderer->Load("model\\Door_VAR01\\Door_Frame.obj");

	m_LeftLeafRenderer = AddComponent<ModelRenderer>();
	m_LeftLeafRenderer->Load("model\\Door_VAR01\\Door_LeafLeft.obj");

	m_RightLeafRenderer = AddComponent<ModelRenderer>();
	m_RightLeafRenderer->Load("model\\Door_VAR01\\Door_LeafRight.obj");

	m_OpenSE = AddComponent<Audio>();
	m_OpenSE->Load("audio\\SE\\sei_ge_doa_open03.mp3");
	SoundManager::RegisterSe(m_OpenSE, 1.0f);
}

void Door::Uninit()
{
	if (m_VertexLayout) { m_VertexLayout->Release(); m_VertexLayout = nullptr; }
	if (m_VertexShader) { m_VertexShader->Release(); m_VertexShader = nullptr; }
	if (m_PixelShader) { m_PixelShader->Release(); m_PixelShader = nullptr; }
	if (m_OpenSE) { SoundManager::Unregister(m_OpenSE); m_OpenSE->Uninit(); }
}

void Door::Update()
{
	if (m_Open && m_OpenT < 1.0f)
	{
		m_OpenT += (1.0f / 60.0f) / kOpenSeconds;
		if (m_OpenT > 1.0f) m_OpenT = 1.0f;
	}

	if (m_IsExit && !m_ClearTriggered && m_OpenT >= 1.0f)
	{
		m_ClearTriggered = true;
		OutputDebugStringA("[Door] exit opened -- CLEAR!\n");
		Manager::ChangeScene<result>(0.5f);
	}
}

void Door::Draw()
{
	Renderer::GetDeviceContext()->IASetInputLayout(m_VertexLayout);
	Renderer::GetDeviceContext()->VSSetShader(m_VertexShader, NULL, 0);
	Renderer::GetDeviceContext()->PSSetShader(m_PixelShader, NULL, 0);

	Vector3 basePos(m_Position.x, 0.0f, m_Position.z);
	float baseYaw = m_Rotation.y;

	XMMATRIX frameWorld = BuildWorld(baseYaw, basePos);
	Renderer::SetWorldMatrix(frameWorld);
	m_FrameRenderer->Draw();

	float leafAngle = -(m_OpenT * (kOpenAngleDeg * (XM_PI / 180.0f)));

	XMMATRIX leftWorld = BuildLeafWorld(kLeftHingeX, baseYaw, leafAngle, basePos);
	Renderer::SetWorldMatrix(leftWorld);
	m_LeftLeafRenderer->Draw();

	XMMATRIX rightWorld = BuildLeafWorld(kRightHingeX, baseYaw, -leafAngle, basePos);
	Renderer::SetWorldMatrix(rightWorld);
	m_RightLeafRenderer->Draw();
}

const char* Door::GetInteractText()
{
	if (m_RequiredKeyId >= 0)
	{
		Player* player = Manager::GetGameObject<Player>();
		if (!(player && player->HasKey(m_RequiredKeyId)))
			return "E ’²‚×‚é";
	}
	return "E ŠJ‚¯‚é";
}

void Door::Interact()
{
	Player* player = Manager::GetGameObject<Player>();

	if (m_RequiredKeyId >= 0)
	{
		if (!(player && player->HasKey(m_RequiredKeyId)))
		{
			OutputDebugStringA("[Door] locked -- needs a key.\n");
			Interact::ShowWarning("Œ®‚ª‚©‚©‚Á‚Ä‚¢‚éB");
			return;
		}
	}

	Open();

	if (m_RequiredKeyId >= 0 && player)
		player->RemoveKey(m_RequiredKeyId);
}

void Door::Open()
{
	if (m_Open) return;
	m_Open = true;
	if (m_OpenSE) m_OpenSE->Play(false);
}
