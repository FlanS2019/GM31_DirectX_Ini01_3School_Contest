#include "main.h"
#include "door.h"
#include "player.h"
#include "manager.h"
#include "Input.h"

namespace
{
	const float kInteractRange = 3.0f; // player must be roughly within one cell
	const float kOpenSeconds = 0.8f;   // time to slide fully open
}

// Door is a Box subclass rather than its own standalone GameObject so it
// automatically gets everything Player.cpp already does for walls for free:
// it shows up in Manager::GetGameObjects<Box>() (the collision loop) and in
// the Z-sorted draw list, with the exact same "GetPosition()=center,
// GetScale()=half-extent" convention Map.cpp and the collision code already
// assume. The only behaviour it needs on top is (a) sometimes not blocking
// (IsBlocking() override, checked by Player.cpp) and (b) sliding open, both
// added here without touching Box's own logic.
void Door::Init()
{
	Box::Init(); // loads model\box.obj / box.mtl -- same look as every wall
}

namespace
{
	// How far (in world axes) it travels to fully clear the doorway, given
	// which way it's sliding. Up uses the slab's own height (slides its
	// full height into the ceiling space). PosX/NegX/PosZ/NegZ use the
	// slab's width along that axis instead, so it slides exactly one
	// slab-width sideways -- into the footprint of whatever wall segment
	// sits next to it. There's no door "pocket" geometry in this blockout,
	// so that's a visual overlap, not a real recess; fine for now.
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
	// Map.cpp sets the door's position via SetPosition() *after*
	// AddGameObject<Door>() runs (same pattern as Box), so Init() is too
	// early to read m_Position -- capture the closed pose the first time
	// Update() actually runs instead.
	if (!m_BaseCaptured)
	{
		m_BasePosition = m_Position;
		m_BaseCaptured = true;
	}

	if (!m_Open)
	{
		Player* player = Manager::GetGameObject<Player>();
		if (player)
		{
			Vector3 direction = player->GetPosition() - m_Position;
			if (direction.length() < kInteractRange && Input::GetKeyTrigger('E'))
			{
				if (m_RequiredKeyId < 0 || player->HasKey(m_RequiredKeyId))
				{
					m_Open = true;
				}
				// else: locked and missing the key -- 'E' does nothing yet.
			}
		}
	}

	if (m_Open && m_OpenT < 1.0f)
	{
		m_OpenT += (1.0f / 60.0f) / kOpenSeconds;
		if (m_OpenT > 1.0f) m_OpenT = 1.0f;

		// Box::Draw() already builds its world matrix from m_Position every
		// frame, so moving m_Position here is all that's needed to animate
		// this -- no Draw() override required, whichever direction it is.
		m_Position = m_BasePosition + OpenOffset(m_SlideDirection, m_Scale) * m_OpenT;
	}

	Box::Update();
}