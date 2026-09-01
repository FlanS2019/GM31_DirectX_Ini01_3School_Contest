#pragma once
#include "gameObject.h"

// Spawns the hospital blockout (walls only, as Box colliders/visuals) from a
// hardcoded grid. The existing Field still provides the floor -- the whole
// grid fits well inside Field's -30..30 footprint, so nothing else changes.
// Tune CELL_SIZE / WALL_HEIGHT and the grid itself in Map.cpp.
class Map : public GameObject
{
public:
	void Init() override;
	void Uninit() override {}
	void Update() override {}
	void Draw() override {}
};