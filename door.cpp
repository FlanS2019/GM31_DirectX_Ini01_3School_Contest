#include "main.h"
#include "door.h"
#include "player.h"
#include "manager.h"
#include "Input.h"

namespace
{
	const float kOpenSeconds = 0.8f; // time to slide fully open
}

void Door::Init()
{
	Box::Init(); // loads model\box.obj / box.mtl -- same look as every wall
}

namespace
{
	Vector3 OpenOffset(Door::SlideDirection dir, const Vector3& scale)
	{
		switch (dir)
		{
		case Door::SlideDirection::PosX: return Vector3(2.0f * scale.x, 0.0f, 0.0f);
		case Door::SlideDirection::NegX: return Vector3(-2.0f * scale.x, 0.0f, 0.0f);
		case Door::SlideDirection::PosZ: return Vector3(0.0f, 0.0f, 2.0f * scale.z);
		case Door::SlideDirection::NegZ: return Vector3(0.0f, 0.0f, -2.0f * scale.z);
		case Door::SlideDirection::Up:
		default:                         return Vector3(0.0f, 2.0f * scale.y, 0.0f);
		}
	}
}

void Door::Update()
{
	if (!m_BaseCaptured)
	{
		m_BasePosition = m_Position;
		m_BaseCaptured = true;
	}


	if (m_Open && m_OpenT < 1.0f)
	{
		m_OpenT += (1.0f / 60.0f) / kOpenSeconds;
		if (m_OpenT > 1.0f) m_OpenT = 1.0f;

		m_Position = m_BasePosition + OpenOffset(m_SlideDirection, m_Scale) * m_OpenT;
	}

	Box::Update();
}

const char* Door::GetInteractText()
{
	if (m_RequiredKeyId >= 0)
	{
		Player* player = Manager::GetGameObject<Player>();
		if (!(player && player->HasKey(m_RequiredKeyId)))
			return "E ’²‚×‚é"; // spec section 4: locked door w/o key
	}
	return "E ŠJ‚¯‚é";
}

void Door::Interact()
{
	if (m_RequiredKeyId >= 0)
	{
		Player* player = Manager::GetGameObject<Player>();
		if (!(player && player->HasKey(m_RequiredKeyId)))
		{
			OutputDebugStringA("[Door] locked -- needs a key.\n");
			return;
		}
	}

	m_Open = true;
}