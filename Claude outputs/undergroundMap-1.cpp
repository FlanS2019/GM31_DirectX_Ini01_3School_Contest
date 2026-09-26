//undergroundMap.cpp
#include "main.h"
#include "undergroundMap.h"
#include "manager.h"
#include "box.h"
#include "lightCase.h"
#include "lightTube.h"
#include "crate.h"
#include "stool.h"
#include "woodenPallet.h"
#include "debris.h"
#include "stainDirt.h"
#include "stainBlood.h"
#include "undergroundWall.h"
#include "generator.h"
#include "shelfRack.h"
#include "pumpUnit.h"
#include "overheadPipe.h"

namespace
{
	// STEP01(stage2): placeholder-room dimensions -- the entrance room, kept
	// exactly as STEP01 built and tested it.
	const float kHalfX = 6.0f;
	const float kHalfZ = 4.0f;
	const float kWallHeight = 3.0f;   // matches Map.cpp's WALL_HEIGHT
	const float kWallThickness = 0.3f; // HALF-thickness -- Box convention is position=center, scale=half-extent
	const float kCeilingThickness = 0.2f; // matches Map.cpp's CEILING_THICKNESS

	// STEP01(stage2): a gap left open in the south wall (the far side from
	// the entrance/spawn point) -- STEP02 continues the map through here.
	const float kGapHalfWidth = 1.5f;

	void SpawnWall(float x, float z, float halfX, float halfZ)
	{
		UndergroundWall* wall = Manager::AddGameObject<UndergroundWall>();
		wall->SetPosition({ x, kWallHeight / 2.0f, z });
		wall->SetScale({ halfX, kWallHeight / 2.0f, halfZ });
	}

	// ------------------------------------------------------------------
	// STEP02(stage2): everything south of the entrance room's gap --
	// corridor, storage room, management room, generator room, item room,
	// and the deepest/climax room. A different, simpler technique than
	// Map.cpp's grid (no ThinWallAxis merging/thinning): every OPEN cell
	// gets a wall spawned on each side that faces a SOLID (or off-grid)
	// neighbor, and nothing at all on sides facing another open cell.
	// Letters beyond '.' (corridor) exist purely to label a cell's room for
	// readability here -- they're all equally "open floor" to the wall/
	// floor/ceiling code below. STEP07 will use them again to decide where
	// to place the key/fuse/documents.
	// ------------------------------------------------------------------

	const int kGridCols = 9;
	const int kGridRows = 19;
	const float kGridCell = 4.0f;
	const float kGridWallHeight = 3.0f;
	const float kGridWallHalfThickness = 0.15f;
	const float kGridCeilingThickness = 0.2f;

	// Row 0's north edge sits flush against the entrance room's south wall
	// (that wall is centered at Z = kHalfZ = 4.0, half-thickness
	// kWallThickness = 0.3, so its far face is at Z = 4.3).
	const float kGridStartZ = 4.3f;

	// The single column that lines up with the entrance room's gap (col 4
	// of 9 -- see the CellCenter formula below: x = 0 when col = COLS/2 -
	// 0.5 = 4.0).
	const int kEntryCol = 4;

	const char* g_UndergroundGrid[kGridRows] =
	{
		"####.####", // row00 -- lines up with the entrance room's south gap
		"####.####", // row01 -- short intro corridor
		"#SSS.####", // row02 -- storage / 倉庫・物置 (west)
		"#SSS.####", // row03
		"####.####", // row04 -- corridor
		"####.MMM#", // row05 -- management / 地下管理室 (east)
		"####.MMM#", // row06
		"####.####", // row07 -- corridor
		"#GGG.####", // row08 -- generator / 発電機室 (west)
		"#GGG.####", // row09
		"####.####", // row10 -- corridor
		"####.III#", // row11 -- item room / アイテム探索用の小部屋 (east)
		"####.III#", // row12
		"####.####", // row13 -- corridor, dread building toward the climax
		"##.....##", // row14 -- widens out before the deepest area
		"#XXXXXXX#", // row15 -- climax / 地下最深部・クライマックスエリア
		"#XXXXXXX#", // row16
		"#XXXXXXX#", // row17
		"#########", // row18 -- south end wall (dead end)
	};

	bool IsGridOpen(int row, int col)
	{
		if (row < 0 || row >= kGridRows || col < 0 || col >= kGridCols) return false;
		return g_UndergroundGrid[row][col] != '#';
	}

	// Same as IsGridOpen(row-1, col), except at row 0's entry column, where
	// "north" is the already-open gap back into the STEP01 entrance room
	// (out-of-bounds would otherwise read as solid rock and wall it shut).
	bool IsGridOpenNorth(int row, int col)
	{
		if (row == 0 && col == kEntryCol) return true;
		return IsGridOpen(row - 1, col);
	}

	Vector3 GridCellCenter(int col, int row)
	{
		float x = (col - kGridCols / 2.0f + 0.5f) * kGridCell;
		float z = kGridStartZ + row * kGridCell + kGridCell * 0.5f;
		return Vector3(x, 0.0f, z);
	}

	// alongX = true for a wall running east-west (its long axis is X, so it
	// needs to be thin in Z); false for one running north-south.
	void SpawnGridWallSegment(float cx, float cz, bool alongX, float halfLength)
	{
		UndergroundWall* wall = Manager::AddGameObject<UndergroundWall>();
		wall->SetPosition({ cx, kGridWallHeight / 2.0f, cz });
		if (alongX)
			wall->SetScale({ halfLength, kGridWallHeight / 2.0f, kGridWallHalfThickness });
		else
			wall->SetScale({ kGridWallHalfThickness, kGridWallHeight / 2.0f, halfLength });
	}

	void SpawnGridWalls()
	{
		float half = kGridCell / 2.0f;

		for (int row = 0; row < kGridRows; row++)
		{
			for (int col = 0; col < kGridCols; col++)
			{
				if (!IsGridOpen(row, col)) continue;

				Vector3 c = GridCellCenter(col, row);

				if (!IsGridOpenNorth(row, col)) SpawnGridWallSegment(c.x, c.z - half, true, half);
				if (!IsGridOpen(row + 1, col)) SpawnGridWallSegment(c.x, c.z + half, true, half);
				if (!IsGridOpen(row, col - 1)) SpawnGridWallSegment(c.x - half, c.z, false, half);
				if (!IsGridOpen(row, col + 1)) SpawnGridWallSegment(c.x + half, c.z, false, half);
			}
		}
	}

	// Ceiling merged per contiguous open run within a row (not a full 2D
	// merge like Map.cpp's -- this map is small enough that row-runs alone
	// keep the Box count reasonable without the extra complexity).
	// STEP02(stage2)-fix: no longer spawns per-cell floor Boxes here -- the
	// box.obj model's UV tiling looked wrong (visible seams/banding) once
	// merged into wide, shallow strips. UndergroundFloor (one big textured
	// quad, texture\UDG.jpg, added alongside Field in the loading screen)
	// covers the whole underground floor instead -- see undergroundFloor.h/cpp.
	void SpawnGridCeilings()
	{
		float half = kGridCell / 2.0f;

		for (int row = 0; row < kGridRows; row++)
		{
			int col = 0;
			while (col < kGridCols)
			{
				if (!IsGridOpen(row, col)) { col++; continue; }

				int runStart = col;
				while (col < kGridCols && IsGridOpen(row, col)) col++;
				int runEnd = col - 1; // inclusive

				Vector3 cStart = GridCellCenter(runStart, row);
				Vector3 cEnd = GridCellCenter(runEnd, row);
				float centerX = (cStart.x + cEnd.x) * 0.5f;
				float halfWidthX = (cEnd.x - cStart.x) * 0.5f + half;

				UndergroundWall* ceiling = Manager::AddGameObject<UndergroundWall>();
				ceiling->SetPosition({ centerX, kGridWallHeight + kGridCeilingThickness / 2.0f, cStart.z });
				ceiling->SetScale({ halfWidthX, kGridCeilingThickness / 2.0f, half });
				ceiling->SetBlocking(false);
			}
		}
	}

	// ------------------------------------------------------------------
	// STEP02(stage2)-detail: room-dressing props -- thin per-grid-cell
	// wrappers around the same decorative GameObject classes Map.cpp
	// already uses for stage 1 (Crate/Stool/WoodenPallet/Debris/StainDirt/
	// StainBlood), so each named underground room reads as a distinct
	// place instead of an empty labelled box. Rotation/offset conventions
	// mirror Map.cpp's own SpawnCrate/SpawnStool/SpawnLeaningPallet/
	// SpawnDebris/SpawnDirtStain/SpawnBloodStain exactly -- none of these
	// classes participate in collision (Player.cpp only checks Box), so
	// they're purely visual and safe to place close to walls/each other.
	// ------------------------------------------------------------------

	void SpawnGridCrate(int col, int row, const Vector3& offset, float yRotation)
	{
		Crate* crate = Manager::AddGameObject<Crate>();
		crate->SetPosition(GridCellCenter(col, row) + offset);
		crate->SetRotation({ 0.0f, yRotation, 0.0f });
	}

	void SpawnGridStool(int col, int row, const Vector3& offset, float yRotation)
	{
		Stool* stool = Manager::AddGameObject<Stool>();
		stool->SetPosition(GridCellCenter(col, row) + offset);
		stool->SetRotation({ 0.0f, yRotation, 0.0f });
	}

	// offset should push the cell center right up against a wall (see the
	// call sites) -- wallYRotation follows Map.cpp's SpawnLeaningPallet
	// convention: XM_PIDIV2 for an east/west-facing wall (offset dominant
	// in X), 0.0f for a north/south-facing one (offset dominant in Z).
	void SpawnGridLeaningPallet(int col, int row, const Vector3& offset, float wallYRotation)
	{
		const float kStandPitch = XM_PIDIV2 - 0.25f; // matches Map.cpp's kLeanFromVertical
		const float kStandLift = 0.48f;

		Vector3 floorPosition = GridCellCenter(col, row) + offset;

		WoodenPallet* pallet = Manager::AddGameObject<WoodenPallet>();
		pallet->SetPosition({ floorPosition.x, floorPosition.y + kStandLift, floorPosition.z });
		pallet->SetRotation({ kStandPitch, wallYRotation, 0.0f });
	}

	void SpawnGridDebris(int col, int row, const Vector3& offset, float yRotation, float scale = 1.0f, float tilt = 0.0f)
	{
		Debris* debris = Manager::AddGameObject<Debris>();
		debris->SetPosition(GridCellCenter(col, row) + offset);
		debris->SetRotation({ tilt, yRotation, 0.0f });
		debris->SetScale({ scale, scale, scale });
	}

	void SpawnGridDirtStain(int col, int row, const Vector3& offset, float yRotation, float scale = 1.0f)
	{
		StainDirt* stain = Manager::AddGameObject<StainDirt>();
		stain->SetPosition(GridCellCenter(col, row) + offset);
		stain->SetRotation({ 0.0f, yRotation, 0.0f });
		stain->SetScale({ scale, 1.0f, scale });
	}

	void SpawnGridBloodStain(int col, int row, const Vector3& offset, float yRotation, float scale = 1.0f)
	{
		StainBlood* stain = Manager::AddGameObject<StainBlood>();
		stain->SetPosition(GridCellCenter(col, row) + offset);
		stain->SetRotation({ 0.0f, yRotation, 0.0f });
		stain->SetScale({ scale, 1.0f, scale });
	}

	void SpawnGridGenerator(int col, int row, const Vector3& offset, float yRotation)
	{
		Generator* generator = Manager::AddGameObject<Generator>();
		generator->SetPosition(GridCellCenter(col, row) + offset);
		generator->SetRotation({ 0.0f, yRotation, 0.0f });
	}

	void SpawnGridShelfRack(int col, int row, const Vector3& offset, float yRotation)
	{
		ShelfRack* shelf = Manager::AddGameObject<ShelfRack>();
		shelf->SetPosition(GridCellCenter(col, row) + offset);
		shelf->SetRotation({ 0.0f, yRotation, 0.0f });
	}

	void SpawnGridPumpUnit(int col, int row, const Vector3& offset, float yRotation)
	{
		PumpUnit* pump = Manager::AddGameObject<PumpUnit>();
		pump->SetPosition(GridCellCenter(col, row) + offset);
		pump->SetRotation({ 0.0f, yRotation, 0.0f });
	}

	// offset.y should already be set near the ceiling (e.g.
	// kGridWallHeight - 0.25f) by the caller -- see overheadPipe.h's
	// comment for why this prop's convention differs from the
	// floor-based ones above.
	void SpawnGridOverheadPipe(int col, int row, const Vector3& offset, float yRotation)
	{
		OverheadPipe* pipe = Manager::AddGameObject<OverheadPipe>();
		pipe->SetPosition(GridCellCenter(col, row) + offset);
		pipe->SetRotation({ 0.0f, yRotation, 0.0f });
	}

	// A pallet lying flat on the floor (as opposed to
	// SpawnGridLeaningPallet's propped-against-a-wall pose) -- matches
	// Map.cpp's SpawnFlatPallet: rotation/pitch/roll all stay at 0 (the
	// model's own resting pose), yRotation just spins it in place.
	void SpawnGridFlatPallet(int col, int row, const Vector3& offset, float yRotation)
	{
		WoodenPallet* pallet = Manager::AddGameObject<WoodenPallet>();
		pallet->SetPosition(GridCellCenter(col, row) + offset);
		pallet->SetRotation({ 0.0f, yRotation, 0.0f });
	}

	// STEP02(stage2)-detail: 「各部屋に小物を配置」 -- each named room gets a
	// small, hand-placed set of props so it reads as a distinct place
	// (storage/management/generator/item/climax) instead of an empty
	// labelled box, using the same decorative classes stage 1 already
	// uses. Kept deliberately light -- a handful of props per room, not
	// dense clutter -- so it doesn't fight the STEP03 "暗所・照明・明暗差"
	// mood or hide the corridor layout.
	void SpawnUndergroundProps()
	{
		// entry corridor -- a couple of faint dirt stains, nothing else
		SpawnGridDirtStain(kEntryCol, 1, Vector3(0.3f, 0.0f, 0.3f), 2.0f, 0.9f);
		SpawnGridDirtStain(kEntryCol, 10, Vector3(-0.2f, 0.0f, 0.4f), 1.2f, 0.8f);
		SpawnGridOverheadPipe(kEntryCol, 7, Vector3(0.0f, kGridWallHeight - 0.25f, 0.0f), XM_PIDIV2); // utility conduit running the length of the spine

		// storage / 倉庫・物置 (rows 2-3, cols 1-3)
		SpawnGridCrate(2, 2, Vector3(-0.3f, 0.0f, -0.4f), 0.6f);
		SpawnGridCrate(3, 2, Vector3(0.2f, 0.0f, -0.3f), 2.4f);
		SpawnGridLeaningPallet(1, 3, Vector3(-1.75f, 0.0f, 0.3f), XM_PIDIV2); // west wall
		SpawnGridDirtStain(2, 3, Vector3(0.4f, 0.0f, 0.5f), 1.1f, 1.3f);
		SpawnGridShelfRack(2, 2, Vector3(0.3f, 0.0f, -1.6f), 0.0f); // against the north wall
		SpawnGridShelfRack(3, 3, Vector3(-0.3f, 0.0f, 0.4f), 1.5f);

		// management / 地下管理室 (rows 5-6, cols 5-7)
		SpawnGridStool(6, 5, Vector3(0.5f, 0.0f, -0.4f), 1.0f);
		SpawnGridStool(7, 6, Vector3(-0.4f, 0.0f, 0.5f), 2.6f);
		SpawnGridDebris(6, 6, Vector3(0.3f, 0.0f, 0.2f), 0.8f, 0.9f);
		SpawnGridBloodStain(7, 5, Vector3(-0.3f, 0.0f, -0.4f), 1.5f, 0.6f); // small, subtle -- something happened here
		SpawnGridShelfRack(6, 6, Vector3(-0.3f, 0.0f, -0.3f), 0.0f);

		// generator / 発電機室 (rows 8-9, cols 1-3) -- a small utility-room cluster,
		// not just one box: the generator itself plus a couple of pump units
		// and an overhead pipe run for the "real machine room" read, and
		// extra debris/a flat broken pallet on top of the existing crates so
		// it reads as run-down/neglected ("ボロボロ") rather than just dark.
		SpawnGridGenerator(2, 8, Vector3(0.0f, 0.0f, 0.3f), 0.6f); // the generator itself -- STEP08 wires up its fuse/E-key gimmick
		SpawnGridPumpUnit(3, 9, Vector3(0.3f, 0.0f, -0.3f), 1.0f);
		SpawnGridPumpUnit(1, 8, Vector3(-0.35f, 0.0f, 0.3f), 2.8f);
		SpawnGridOverheadPipe(2, 8, Vector3(0.0f, kGridWallHeight - 0.25f, 0.3f), 0.0f);
		SpawnGridCrate(1, 8, Vector3(0.3f, 0.0f, -0.3f), 1.8f);
		SpawnGridCrate(2, 9, Vector3(-0.2f, 0.0f, 0.4f), 3.0f);
		SpawnGridLeaningPallet(1, 9, Vector3(-1.75f, 0.0f, -0.3f), XM_PIDIV2); // west wall
		SpawnGridFlatPallet(3, 8, Vector3(0.3f, 0.0f, 0.35f), 1.2f); // broken, left where it fell
		SpawnGridDebris(3, 8, Vector3(-0.3f, 0.0f, -0.2f), 2.2f, 1.1f);
		SpawnGridDebris(1, 9, Vector3(0.3f, 0.0f, -0.4f), 0.5f, 1.0f);

		// item room / アイテム探索用の小部屋 (rows 11-12, cols 5-7)
		SpawnGridCrate(6, 11, Vector3(-0.3f, 0.0f, 0.4f), 0.5f);
		SpawnGridStool(7, 12, Vector3(0.4f, 0.0f, -0.5f), 2.0f);
		SpawnGridDirtStain(6, 12, Vector3(0.2f, 0.0f, 0.3f), 0.4f, 1.0f);
		SpawnGridShelfRack(6, 11, Vector3(0.3f, 0.0f, -0.4f), 0.0f);

		// widened area right before the climax (row 14) -- a hint that something's off, nothing more
		SpawnGridDebris(2, 14, Vector3(0.3f, 0.0f, -0.2f), 1.3f, 1.0f);
		SpawnGridDebris(6, 14, Vector3(-0.2f, 0.0f, 0.3f), 0.4f, 0.85f);

		// climax / 地下最深部・クライマックスエリア (rows 15-17) -- kept almost
		// empty on purpose (flashlight-only, spec section 5-3 / 15); just one
		// stain near the center as the only hint of dread down here.
		SpawnGridBloodStain(kEntryCol, 16, Vector3(0.0f, 0.0f, 0.0f), 0.0f, 1.4f);
	}

	void SpawnGridLightFixture(int col, int row, bool isLit, bool flicker)
	{
		const float kFaceDownPitch = XM_PIDIV2;
		const float kTubeDrop = 0.12f;
		const float kTubeSideOffset = 0.05f;

		Vector3 position = GridCellCenter(col, row) + Vector3(0.0f, kGridWallHeight - 0.05f, 0.0f);

		LightCase* lightCase = Manager::AddGameObject<LightCase>();
		lightCase->SetPosition(position);
		lightCase->SetRotation({ kFaceDownPitch, 0.0f, 0.0f });

		if (!isLit) return;

		for (int i = 0; i < 2; i++)
		{
			float side = (i == 0) ? -1.0f : 1.0f;

			LightTube* lightTube = Manager::AddGameObject<LightTube>();
			lightTube->SetPosition({ position.x + side * kTubeSideOffset, position.y - kTubeDrop, position.z });
			lightTube->SetRotation({ kFaceDownPitch, 0.0f, 0.0f });
			lightTube->SetFlicker(flicker);
		}
	}

	void SpawnGridDisplacedTube(int col, int row)
	{
		const float kFaceDownPitch = XM_PIDIV2;
		const float kTubeDrop = 0.12f;
		const float kExtraDrop = 0.15f;  // dropped further than a normal tube -- hanging loose
		const float kExtraTilt = 0.5f;   // added to both pitch and roll -- crooked, not level

		Vector3 position = GridCellCenter(col, row) + Vector3(0.0f, kGridWallHeight - 0.05f, 0.0f);

		LightCase* lightCase = Manager::AddGameObject<LightCase>();
		lightCase->SetPosition(position);
		lightCase->SetRotation({ kFaceDownPitch, 0.0f, 0.0f });

		// only ONE tube (not the usual pair), dropped further and tilted, and
		// always flickering -- mirrors Map.cpp's SpawnDisplacedTube() (stage 1's
		// "外れかかっている" fixture).
		LightTube* lightTube = Manager::AddGameObject<LightTube>();
		lightTube->SetPosition({ position.x, position.y - (kTubeDrop + kExtraDrop), position.z });
		lightTube->SetRotation({ kFaceDownPitch + kExtraTilt, 0.0f, kExtraTilt });
		lightTube->SetFlicker(true);
	}

	// STEP03(stage2)-fix: 「照明の数をもう少し増やしたいのと廃墟のステージと同じ
	// く、光っていないところとはずれているところを再現してほしい」-- more total
	// fixtures than the original curated 6, and now using the same three-way mix
	// as Map.cpp's stage 1 (点灯 / 消灯 / ずれている・外れかかっている), not just
	// lit-vs-unlit. Still hand-placed rather than looping over every open cell --
	// this stays deliberately sparser and more uneven than stage 1's denser
	// corridor-wide checkerboard, on purpose ("暗所・照明・明暗差"). The generator
	// room still stays dark (power's out until STEP08) and the climax room still
	// has zero fixtures (flashlight only, spec section 5-3 / 15) -- neither of
	// those design choices changes here.
	void SpawnUndergroundLights()
	{
		SpawnGridLightFixture(kEntryCol, 1, true, false);  // entry corridor -- calm, steady

		SpawnGridLightFixture(2, 2, true, true);           // storage -- flickering
		SpawnGridLightFixture(1, 3, false, false);         // storage, far corner -- unlit

		SpawnGridDisplacedTube(kEntryCol, 4);              // corridor, storage->management -- hanging loose

		SpawnGridLightFixture(6, 5, true, false);          // management -- steady, needs to be readable
		SpawnGridDisplacedTube(5, 6);                      // management, second fixture -- crooked

		SpawnGridLightFixture(kEntryCol, 7, false, false); // corridor, management->generator -- unlit

		SpawnGridLightFixture(2, 8, false, false);         // generator -- dark/broken until STEP08

		SpawnGridLightFixture(kEntryCol, 10, true, true);  // corridor, generator->item room -- flickering

		SpawnGridLightFixture(6, 11, true, true);          // item room -- flickering
		SpawnGridLightFixture(7, 12, false, false);        // item room, second fixture -- unlit

		SpawnGridLightFixture(kEntryCol, 13, true, true);  // last corridor before the climax -- flickering, building dread
		SpawnGridDisplacedTube(kEntryCol, 14);             // widened area right before the climax -- crooked, one last warning
		// rows 15-17 (climax room): intentionally no fixture at all.
	}

	void BuildUndergroundGrid()
	{
		SpawnGridWalls();
		SpawnGridCeilings();
		SpawnUndergroundLights();
		SpawnUndergroundProps();
	}
}

void UndergroundMap::Init()
{
	// north wall -- behind the spawn point, the way back up to the entrance door
	SpawnWall(0.0f, -kHalfZ, kHalfX + kWallThickness, kWallThickness);

	// east / west walls
	SpawnWall(kHalfX, 0.0f, kWallThickness, kHalfZ + kWallThickness);
	SpawnWall(-kHalfX, 0.0f, kWallThickness, kHalfZ + kWallThickness);

	// south wall, split around a gap -- the grid below continues the map through here
	{
		float gapOuterHalf = (kHalfX - kGapHalfWidth) / 2.0f;
		float gapOuterCenter = kGapHalfWidth + gapOuterHalf;
		SpawnWall(gapOuterCenter, kHalfZ, gapOuterHalf, kWallThickness);
		SpawnWall(-gapOuterCenter, kHalfZ, gapOuterHalf, kWallThickness);
	}

	// ceiling -- same position/scale/SetBlocking(false) convention as
	// Map.cpp's SpawnMergedCeiling()
	UndergroundWall* ceiling = Manager::AddGameObject<UndergroundWall>();
	ceiling->SetPosition({ 0.0f, kWallHeight + kCeilingThickness / 2.0f, 0.0f });
	ceiling->SetScale({ kHalfX + kWallThickness, kCeilingThickness / 2.0f, kHalfZ + kWallThickness });
	ceiling->SetBlocking(false);

	// STEP02/03(stage2): the corridor + rooms + lights beyond the gap.
	BuildUndergroundGrid();
}

Vector3 UndergroundMap::GetSpawnPosition()
{
	// just south of the north wall, roughly centered -- "just through the
	// entrance door, at the top of the stairs" per the spec's section 3.
	return Vector3(0.0f, 0.0f, -kHalfZ + 1.5f);
}

