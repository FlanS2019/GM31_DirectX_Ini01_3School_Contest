#pragma once
#include "gameObject.h"

// STEP01(stage2): a small placeholder room (just past the entrance door) --
// enough to satisfy STEP01's own completion condition ("地下へ移動できる").
// STEP02 ("地下マップ制作") replaces this file's contents with the real
// corridor/room layout from the spec; this deliberately does NOT port
// Map.cpp's full grid/ThinWallAxis wall-generation system, since everything
// here is meant to be grown into (or wholesale replaced by) STEP02 anyway --
// a handful of hardcoded Box walls is enough for one placeholder room.
class UndergroundMap : public GameObject
{
public:
	void Init() override;
	void Uninit() override {}
	void Update() override {}
	void Draw() override {}

	// STEP01(stage2): where Underground::Init() puts the player, both on
	// first entry and on every future respawn (re-entering this scene). A
	// static/free function, not tied to an instance, so Underground::Init()
	// can call it before this object even exists yet -- same reasoning as
	// GameSettings' static accessors.
	static Vector3 GetSpawnPosition();
};
