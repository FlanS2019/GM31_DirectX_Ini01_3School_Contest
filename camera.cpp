#include "main.h"
#include "camera.h"
#include "renderer.h"
#include "manager.h"
#include "player.h"
#include "Input.h"

void Camera::Init()
{
	m_Position = { 0, 5, -20 };
	//m_Yaw = 0.0f;    // 追加
	//m_Pitch = 0.3f;  // 追加
	//m_Distance = 8.0f; // 追加
}

void Camera::Uninit()
{
}

void Camera::Update()
{
	Player* player = Manager::GetGameObject<Player>();
	Vector3 playerPos = player->GetPosition();
	float playerYaw = player->GetRotation().y; // プレイヤーの向き（Yaw）を取得

	float t = 0.1f;
	m_Target = m_Target * (1.0f - t) + (playerPos + Vector3(0.0f, 2.0f, 0.0f)) * t;

	const float tXZ = 0.1f;  // 水平方向の追従速度
	const float tY = 0.04f; // 垂直方向はより緩やかに（ジャンプの影響を抑える）

	m_Target.x = m_Target.x * (1.0f - tXZ) + m_Target.x * tXZ;
	m_Target.z = m_Target.z * (1.0f - tXZ) + m_Target.z * tXZ;
	m_Target.y = m_Target.y * (1.0f - tY) + m_Target.y * tY;
	// プレイヤーの背後にカメラを配置（プレイヤーの向きに応じて回り込む）
	const float distance = 8.0f;  // プレイヤーとの水平距離
	const float height = 4.0f;    // プレイヤーの上に何m上げるか

	Vector3 desiredPosition = playerPos
		+ Vector3(
			-sinf(playerYaw) * distance,
			height,
			-cosf(playerYaw) * distance
		);

	m_Position = m_Position * (1.0f - t) + desiredPosition * t;
}

void Camera::Draw()
{
	//projection行列の作成
	XMMATRIX projection = XMMatrixPerspectiveFovLH
	(1, (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 1000.0f);

	Renderer::SetProjectionMatrix(projection);

	XMFLOAT3 up = XMFLOAT3(0, 1, 0);
	XMFLOAT3 pos = { m_Position.x, m_Position.y, m_Position.z };
	XMFLOAT3 target = { m_Target.x, m_Target.y, m_Target.z };
	m_ViewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&pos),
		XMLoadFloat3(&target), XMLoadFloat3(&up));

	Renderer::SetViewMatrix(m_ViewMatrix);
}