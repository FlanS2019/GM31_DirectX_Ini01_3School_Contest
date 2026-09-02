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

void Door::Update()
{
	// Map.cpp sets the door's position via SetPosition() *after*
	// AddGameObject<Door>() runs (same pattern as Box), so Init() is too
	// early to read m_Position -- capture the closed-position Y the first
	// time Update() actually runs instead.
	if (!m_BaseYCaptured)
	{
		m_BaseY = m_Position.y;
		m_BaseYCaptured = true;
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
				// (A UI prompt/SE for "it's locked" is a nice follow-up once
				// there's a HUD to put it on.)
			}
		}
	}

	if (m_Open && m_OpenT < 1.0f)
	{
		m_OpenT += (1.0f / 60.0f) / kOpenSeconds;
		if (m_OpenT > 1.0f) m_OpenT = 1.0f;

		// Slide the whole slab straight up out of the doorway. Box::Draw()
		// already builds its world matrix from m_Position every frame, so
		// moving m_Position.y here is all that's needed to animate it --
		// no Draw() override required.
		m_Position.y = m_BaseY + m_OpenT * (2.0f * m_Scale.y);
	}

	Box::Update();
}
