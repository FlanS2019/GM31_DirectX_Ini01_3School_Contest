//undergroundMap.cpp
#include "main.h"
#include "undergroundMap.h"
#include "manager.h"
#include "box.h"

namespace
{
	// STEP01(stage2): placeholder-room dimensions. STEP02 replaces this
	// whole file's contents with the real underground layout from the spec --
	// see the comment in undergroundMap.h.
	const float kHalfX = 6.0f;
	const float kHalfZ = 4.0f;
	const float kWallHeight = 3.0f;   // matches Map.cpp's WALL_HEIGHT
	const float kWallThickness = 0.3f; // HALF-thickness -- Box convention is position=center, scale=half-extent
	const float kCeilingThickness = 0.2f; // matches Map.cpp's CEILING_THICKNESS

	// STEP01(stage2): a gap left open in the south wall (the far side from
	// the entrance/spawn point) -- the natural place for STEP02 to continue
	// the map deeper underground.
	const float kGapHalfWidth = 1.5f;

	void SpawnWall(float x, float z, float halfX, float halfZ)
	{
		Box* wall = Manager::AddGameObject<Box>();
		wall->SetPosition({ x, kWallHeight / 2.0f, z });
		wall->SetScale({ halfX, kWallHeight / 2.0f, halfZ });
	}
}

void UndergroundMap::Init()
{
	// north wall -- behind the spawn point, the way back up to the entrance door
	SpawnWall(0.0f, -kHalfZ, kHalfX + kWallThickness, kWallThickness);

	// east / west walls
	SpawnWall(kHalfX, 0.0f, kWallThickness, kHalfZ + kWallThickness);
	SpawnWall(-kHalfX, 0.0f, kWallThickness, kHalfZ + kWallThickness);

	// south wall, split around a gap -- STEP02 continues the map through here
	{
		float gapOuterHalf = (kHalfX - kGapHalfWidth) / 2.0f;
		float gapOuterCenter = kGapHalfWidth + gapOuterHalf;
		SpawnWall(gapOuterCenter, kHalfZ, gapOuterHalf, kWallThickness);
		SpawnWall(-gapOuterCenter, kHalfZ, gapOuterHalf, kWallThickness);
	}

	// ceiling -- same position/scale/SetBlocking(false) convention as
	// Map.cpp's SpawnMergedCeiling()
	Box* ceiling = Manager::AddGameObject<Box>();
	ceiling->SetPosition({ 0.0f, kWallHeight + kCeilingThickness / 2.0f, 0.0f });
	ceiling->SetScale({ kHalfX + kWallThickness, kCeilingThickness / 2.0f, kHalfZ + kWallThickness });
	ceiling->SetBlocking(false);
}

Vector3 UndergroundMap::GetSpawnPosition()
{
	// just south of the north wall, roughly centered -- "just through the
	// entrance door, at the top of the stairs" per the spec's section 3.
	return Vector3(0.0f, 0.0f, -kHalfZ + 1.5f);
}
