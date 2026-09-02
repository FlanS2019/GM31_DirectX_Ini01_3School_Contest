#pragma once

#include "box.h"

// STEP2: a wall segment that can be opened, either by a player carrying the
// right key (m_RequiredKeyId) or externally by a Switch (Open()). Reuses
// Box's model/collision/Z-sort plumbing -- see the comments in door.cpp for
// why it's a Box subclass instead of a standalone GameObject.
class Door : public Box
{
private:
	int m_RequiredKeyId = -1; // -1 = no key needed, 'E' alone opens it
	bool m_Open = false;      // true once opening has been triggered
	float m_OpenT = 0.0f;     // 0 = closed, 1 = fully open
	float m_BaseY = 0.0f;     // Y position the door was placed at (closed)
	bool m_BaseYCaptured = false;

public:
	void Init()override;
	void Update()override;

	bool IsBlocking() const override { return m_OpenT < 1.0f; }

	// Map.cpp calls this right after AddGameObject<Door>() to lock it to a
	// specific key id; leave untouched for a door that only needs 'E'.
	void SetRequiredKey(int keyId) { m_RequiredKeyId = keyId; }

	// Switch calls this to open the door regardless of key/distance.
	// Safe to call repeatedly (no-op once already open).
	void Open() { m_Open = true; }

	bool IsOpen() const { return m_OpenT >= 1.0f; }
};
