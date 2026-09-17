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

	// STEP: 扉(LeafLeft/LeafRight)のローカル座標には、分割時に枠の中心を
	// 原点に揃えた都合上、蝶番(ヒンジ)位置ぶんのオフセットがすでに
	// 焼き込まれている。以前はBuildWorld()と同じ「回転してから平行移動」
	// だけで描いていたため、そのオフセットが二重にかかってしまい、
	// leafAngle=0(閉じているはず)の状態でも扉が本来の位置・向きから
	// 大きくズレて描画されていた -- これが「ドアが常に開いて見える」
	// 「開く方向がおかしい」の原因。
	// 正しい蝶番回転は「ヒンジをいったん原点に戻す平行移動 -> 追加角度
	// ぶん回転 -> 回転後のワールド上のヒンジ位置へ平行移動」の3段階。
	// こうすると leafAngle=0 のときはBuildWorld(baseYaw, basePos)と
	// ぴったり同じ変換になり(=枠と同じ平面に収まる=閉じて見える)、
	// leafAngle!=0 のときはヒンジを軸に正しく開閉するようになる。
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

	// STEP21: "ドアを開ける音"
	m_OpenSE = AddComponent<Audio>();
	m_OpenSE->Load("audio\\SE\\sei_ge_doa_open03.mp3");
	SoundManager::RegisterSe(m_OpenSE, 1.0f); // STEP24: 設定画面のSE音量スライダーを反映
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

	// STEP: 開く方向がおかしいと言われたため、以前と符号を反転(奥に
	// 開く向きへ)。まだ逆だったら、この2行の符号をもう一度反転するだけ
	// で直る。
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
			return "E 調べる";
	}
	return "E 開ける";
}

void Door::Interact()
{
	Player* player = Manager::GetGameObject<Player>();

	if (m_RequiredKeyId >= 0)
	{
		if (!(player && player->HasKey(m_RequiredKeyId)))
		{
			Interact::ShowWarning("鍵がかかっている。");
			return;
		}
	}

	// STEP21: routed through Open() (below) instead of setting m_Open here
	// directly, so the open SE plays from ONE place shared with
	// Switch::Interact()'s direct Open() call -- see door.h's Open() comment.
	Open();

	// STEP14: hotbar "使ったら消える" request -- the key that unlocked this
	// door is spent now, so drop it from Player's inventory mask; the
	// hotbar in interact.cpp only draws ids HasKey() still returns true
	// for, so this makes the slot disappear the instant the door opens.
	if (m_RequiredKeyId >= 0 && player)
		player->RemoveKey(m_RequiredKeyId);
}

// STEP21: centralizes "door starts opening" so both a direct player
// interact (above) and Switch::Interact()'s direct call both get the SE,
// and neither path can fire it twice -- m_Open guards against a second
// call restarting the sound (e.g. if Interact() somehow ran again).
void Door::Open()
{
	if (m_Open) return;
	m_Open = true;
	if (m_OpenSE) m_OpenSE->Play(false);
}
