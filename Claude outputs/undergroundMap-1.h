#pragma once
#include "gameObject.h"

// STEP01(stage2): the small entrance room just past the underground door --
// kept exactly as STEP01 built and tested it.
// STEP02(stage2): everything beyond that room's south gap -- corridor,
// storage room, management room, generator room, item room, and the
// deepest/climax room -- built as its own compact grid (see undergroundMap.cpp),
// separate from the STEP01 room's hardcoded boxes and from Map.cpp's
// grid/ThinWallAxis merging system. Every open cell gets a wall spawned on
// each side that faces a solid (or off-grid) neighbor and nothing on sides
// facing another open cell -- simple enough to be low-risk, and enough to
// enclose an arbitrary room/corridor shape correctly with no per-room wall
// math.
// STEP03(stage2): light fixtures (LightCase/LightTube) placed sparsely
// across that same grid -- see SpawnUndergroundLights() in the .cpp -- plus
// a darker global Light ambient/diffuse set from UndergroundLoadingScreen's
// own Light-spawn step, so only this scene gets the darker look.
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
