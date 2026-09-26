#pragma once

#include "box.h"

// STEP02(stage2)-detail: a Box that swaps box.obj's shared ruins texture
// (broken_wall.jpg, also used by every stage 1 wall/ceiling/door-frame) for
// a plain dark stone material of its own (model\undergroundWall.obj/.mtl),
// so the underground reads as a different place instead of reusing stage
// 1's exact wall look. Everything else -- collision (Player.cpp only
// checks Box, and Manager::GetGameObjects<Box>() finds subclasses via
// dynamic_cast, so this still blocks the player normally), IsCullable(),
// and the position/scale/UV-tiling convention in Box::Draw() -- is
// inherited unchanged from Box; only GetModelPath() differs.
class UndergroundWall : public Box
{
public:
	const char* GetModelPath() const override { return "model\\undergroundWall.obj"; }
};
