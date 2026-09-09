#include "main.h"
#include "Map.h"
#include "manager.h"
#include "box.h"
#include "door.h"
#include "key.h"
#include "switch.h"
#include "item.h"
#include "itemBox.h"
#include "lightTube.h"
#include "lightCase.h"
#include "particle.h"

namespace
{
	const int COLS = 12;
	const int ROWS = 11;
	const float CELL_SIZE = 4.0f;   // shrink this later to tighten corridors/rooms
	const float WALL_HEIGHT = 3.0f;
	const float CEILING_THICKNESS = 0.2f; // STEP10: how thick the ceiling slab reads as, purely cosmetic

	// STEP10: how thin a wall's OPEN-facing side recedes -- see PlaceWall()
	// below. 1.0 = the original full-cube look (no shrink); lower = thinner.
	// Doors are NOT thinned by this (see PlaceWall()'s comment), only plain
	// '#' walls.
	const float WALL_THICKNESS_SCALE = 0.3f;

	// STEP13: X (switch) moved from room N into room A (against its west
	// wall) and F (item box) moved from room N into room B -- room N used
	// to hold P/X/F all at once while A only had the key and C only had
	// one item, so items/gimmicks read as "scattered" now instead of piled
	// into a single room.
	const char* g_Grid[ROWS] =
	{
		"############",
		"#AAA#..#CRC#",
		"#AKA...DCCC#",
		"#XAA#..#CCC#",
		"#####..#####",
		"#BMB#..#NPN#",
		"#BBBG...NNN#",
		"#FBB#..#NNN#",
		"#####..#####",
		"#####..#####",
		"#####E######",
	};

	Vector3 CellCenter(int col, int row)
	{
		float x = (col - COLS / 2.0f + 0.5f) * CELL_SIZE;
		float z = (row - ROWS / 2.0f + 0.5f) * CELL_SIZE;
		return Vector3(x, 0.0f, z);
	}

	// STEP10: true if (row,col) is a wall/door cell -- i.e. something Box-
	// shaped occupying its whole cell -- false for floor/props/out-of-grid.
	// Out-of-bounds counts as solid so an edge wall's outward-facing side
	// (which nobody can ever see past) doesn't recede either; it just
	// doesn't matter which way that particular default falls.
	bool IsSolidCell(int row, int col)
	{
		if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return true;
		char c = g_Grid[row][col];
		return c == '#' || c == 'D' || c == 'G' || c == 'E';
	}

	// True only for a plain open floor cell ('.') -- used to decide where
	// to put ceiling light fixtures. Out of bounds counts as NOT open.
	bool IsOpenFloor(int row, int col)
	{
		if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return false;
		return g_Grid[row][col] == '.';
	}

	// STEP10: `half` is a wall's un-thinned half-extent on one axis;
	// `openNeg`/`openPos` say whether the neighbor on the negative/positive
	// side of that axis is open space. Returns the half-extent to actually
	// use, and how far to shift the box's center on that axis so a face
	// bordering another solid wall/door cell stays exactly at the cell's
	// original boundary (flush -- no crack against that neighbor), while a
	// face bordering open space recedes inward by WALL_THICKNESS_SCALE.
	void ThinWallAxis(float half, bool openNeg, bool openPos, float& outOffset, float& outHalf)
	{
		if (!openNeg && !openPos)
		{
			// Solid neighbor on both sides (mid-run of a straight wall) --
			// don't shrink at all, or BOTH neighbors would show a crack.
			outOffset = 0.0f;
			outHalf = half;
			return;
		}

		outHalf = half * WALL_THICKNESS_SCALE;

		if (openNeg && openPos)
		{
			// Free-standing on both sides (an isolated pillar cell) --
			// shrink evenly.
			outOffset = 0.0f;
		}
		else if (openPos)
		{
			// Flush against a neighbor on the negative side -- keep that
			// face put, recede only the open (positive) side.
			outOffset = -(half - outHalf);
		}
		else // openNeg
		{
			// Flush against a neighbor on the positive side -- recede only
			// the open (negative) side.
			outOffset = (half - outHalf);
		}
	}

	// STEP10: places a plain '#' wall's ACTUAL collision+visual box
	// (SetPosition()/SetScale() directly -- box.cpp's Draw() just renders
	// whatever those are, no separate visual-only shrink anymore), thinned
	// on whichever sides border open space and left flush/full-extent on
	// sides bordering another wall/door cell. This is deliberately NOT used
	// for Door: Door::OpenOffset() (door.cpp) derives how far a door slides
	// open from its own m_Scale, so thinning that would make a door stop
	// short of fully clearing its doorway -- doors keep the original
	// full-cell size, collision and visual both.
	void PlaceWall(Box* wall, const Vector3& center, int row, int col)
	{
		float offsetX, halfX, offsetZ, halfZ;
		ThinWallAxis(CELL_SIZE / 2.0f, !IsSolidCell(row, col - 1), !IsSolidCell(row, col + 1), offsetX, halfX);
		ThinWallAxis(CELL_SIZE / 2.0f, !IsSolidCell(row - 1, col), !IsSolidCell(row + 1, col), offsetZ, halfZ);

		wall->SetPosition({ center.x + offsetX, WALL_HEIGHT / 2.0f, center.z + offsetZ });
		wall->SetScale({ halfX, WALL_HEIGHT / 2.0f, halfZ });
	}

	// STEP9/10: one ceiling light fixture (LightCase housing + LightTube
	// inside it) at `position`, both facing the same yRotation. See
	// lightTube.h/lightCase.h -- both models' scale/rotation are starting
	// guesses (no way to preview the render from here), so treat this
	// whole function's numbers as things to nudge once visible in-game.
	void SpawnLightFixture(const Vector3& position, float yRotation)
	{
		// Both models turned out to be near-flat (almost zero local Z
		// thickness) rather than a round tube/case -- a pure Y-axis spin
		// (yRotation below) only swivels that flat face to point sideways,
		// never up or down. This extra X-axis (pitch) rotation walks the
		// local Z axis onto world -Y so the flat/glowing face points down
		// at the floor like a real ceiling fixture. The SIGN is a guess --
		// if it ends up facing the ceiling instead of the floor, flip it
		// to -XM_PIDIV2.
		const float kFaceDownPitch = XM_PIDIV2;

		// STEP10: the first pass put the tube at the exact same spot as
		// the case, which (going by the one screenshot available so far)
		// read as "no visible tube" -- it was likely sitting fully inside/
		// behind the case's own geometry. Dropping it a little further
		// down (world -Y, independent of the pitch/yaw above) is a simple
		// way to make it hang out from the housing as its own visible
		// piece. Guess, not a measurement -- adjust freely.
		const float kTubeDrop = 0.05f;

		LightCase* lightCase = Manager::AddGameObject<LightCase>();
		lightCase->SetPosition(position);
		lightCase->SetRotation({ kFaceDownPitch, yRotation, 0.0f });

		LightTube* lightTube = Manager::AddGameObject<LightTube>();
		lightTube->SetPosition({ position.x, position.y - kTubeDrop, position.z });
		lightTube->SetRotation({ kFaceDownPitch, yRotation, 0.0f });
	}

	// STEP13: one ambient dust-mote emitter at a room's center -- see
	// Particle::SetAmbientMode(). radius/height are sized to roughly cover
	// a 3x3-cell room (this map's rooms are all that size); count is kept
	// modest since every room gets its own emitter and draw cost adds up.
	void SpawnDustMotes(const Vector3& roomCenter)
	{
		Particle* dust = Manager::AddGameObject<Particle>();
		dust->SetPosition(roomCenter);
		dust->SetScale({ 0.06f, 0.06f, 0.06f });
		dust->SetAmbientMode(CELL_SIZE * 1.4f, WALL_HEIGHT - 0.3f, 25);
	}
}

void Map::Init()
{
	Door* gimmickDoor = nullptr;

	for (int row = 0; row < ROWS; row++)
	{
		for (int col = 0; col < COLS; col++)
		{
			char cell = g_Grid[row][col];
			Vector3 center = CellCenter(col, row);

			switch (cell)
			{
			case '#':
			{
				Box* wall = Manager::AddGameObject<Box>();
				PlaceWall(wall, center, row, col);
				break;
			}
			case 'D':
			{
				Door* door = Manager::AddGameObject<Door>();
				door->SetPosition({ center.x, WALL_HEIGHT / 2.0f, center.z });
				door->SetScale({ CELL_SIZE / 2.0f, WALL_HEIGHT / 2.0f, CELL_SIZE / 2.0f });
				door->SetRequiredKey(0); // matches the 'K' key below
				door->SetSlideDirection(Door::SlideDirection::NegZ);
				break;
			}
			case 'G':
			{
				Door* door = Manager::AddGameObject<Door>();
				door->SetPosition({ center.x, WALL_HEIGHT / 2.0f, center.z });
				door->SetScale({ CELL_SIZE / 2.0f, WALL_HEIGHT / 2.0f, CELL_SIZE / 2.0f });
				door->SetSlideDirection(Door::SlideDirection::NegZ);

				gimmickDoor = door;
				break;
			}
			case 'K':
			{
				Key* key = Manager::AddGameObject<Key>();
				key->SetPosition({ center.x, key->GetPosition().y, center.z });
				key->SetKeyId(0);
				break;
			}
			case 'X':
			{
				Switch* sw = Manager::AddGameObject<Switch>();
				// STEP13: was Y=0 in the middle of the room -- box.cpp's draw
				// convention (position=center, shifted down by m_Scale.y) put
				// that half-buried in the floor and nowhere near a wall. Now
				// raised to hand height and offset toward this cell's west wall
				// (one cell over, at col0) so it actually reads as wall-mounted.
				const float kWallOffset = CELL_SIZE / 2.0f - 0.35f;
				const float kMountHeight = 1.2f;
				sw->SetPosition({ center.x - kWallOffset, kMountHeight, center.z });
				sw->SetTargetDoor(gimmickDoor);
				break;
			}
			case 'P':
			{
				Item* item = Manager::AddGameObject<Item>();
				item->SetPosition({ center.x, item->GetPosition().y, center.z });
				item->SetItemId(1);
				item->SetDisplayName("å√Ç¢é ê^");
				break;
			}
			case 'R':
			{
				Item* item = Manager::AddGameObject<Item>();
				item->SetPosition({ center.x, item->GetPosition().y, center.z });
				item->SetItemId(2);
				item->SetDisplayName("êFÇ†ÇπÇΩéËéÜ"); // STEP13: was a hospital "diagnosis record" -- swapped for something a ruins theme actually fits
				break;
			}
			case 'M':
			{
				Item* item = Manager::AddGameObject<Item>();
				item->SetPosition({ center.x, item->GetPosition().y, center.z });
				item->SetItemId(3);
				item->SetDisplayName("ã‡ëÆïîïi");
				break;
			}
			case 'F':
			{
				ItemBox* box = Manager::AddGameObject<ItemBox>();
				box->SetPosition({ center.x, 0.0f, center.z });
				box->SetRequiredItemIds(1, 2, 3); // P / R / M above
				box->SetFinalKeyId(4);
				break;
			}
			case 'E':
			{
				Door* door = Manager::AddGameObject<Door>();
				door->SetPosition({ center.x, WALL_HEIGHT / 2.0f, center.z });
				door->SetScale({ CELL_SIZE / 2.0f, WALL_HEIGHT / 2.0f, CELL_SIZE / 2.0f });
				door->SetRequiredKey(4); // final key, from the 'F' box above
				door->SetIsExit(true);   // finishing its open animation triggers CLEAR -- see door.cpp
				door->SetSlideDirection(Door::SlideDirection::PosZ); // slides further south, away from the map
				break;
			}
			default:
				break; // plain floor; Field is the floor mesh
			}
		}
	}

	// STEP10: a thin ceiling slab over every cell (walls included -- their
	// tops sit exactly at WALL_HEIGHT, flush with the ceiling's underside,
	// so covering wall cells too is harmless and keeps this one simple
	// loop). One Box per cell rather than a single map-spanning slab so
	// box.mtl's texture tiles once per cell like the walls do, instead of
	// stretching across the whole ceiling. Not blocking (SetBlocking(false))
	// -- see box.h's SetBlocking() comment.
	for (int row = 0; row < ROWS; row++)
	{
		for (int col = 0; col < COLS; col++)
		{
			Vector3 center = CellCenter(col, row);

			Box* ceiling = Manager::AddGameObject<Box>();
			ceiling->SetPosition({ center.x, WALL_HEIGHT + CEILING_THICKNESS / 2.0f, center.z });
			ceiling->SetScale({ CELL_SIZE / 2.0f, CEILING_THICKNESS / 2.0f, CELL_SIZE / 2.0f });
			ceiling->SetBlocking(false);
		}
	}

	// STEP10: a light fixture over every open corridor cell, instead of
	// just the 2 hand-picked spots from the first pass -- "regular
	// interval" falls out naturally from covering every '.' cell in the
	// grid. yRotation is picked per-cell: an East-West-only corridor
	// segment gets 0 (the tube's default local-X long axis already matches
	// world X); anything else (a North-South run, or a junction/room where
	// both directions are open) gets XM_PIDIV2. Best-effort heuristic --
	// there's no way to preview the actual render from here, so if a run
	// of fixtures looks rotated wrong once this is visible in-game, this
	// loop is the one place to fix it.
	for (int row = 0; row < ROWS; row++)
	{
		for (int col = 0; col < COLS; col++)
		{
			if (!IsOpenFloor(row, col)) continue;

			bool northSouthOpen = IsOpenFloor(row - 1, col) || IsOpenFloor(row + 1, col);
			bool eastWestOpen = IsOpenFloor(row, col - 1) || IsOpenFloor(row, col + 1);
			float yRotation = (eastWestOpen && !northSouthOpen) ? 0.0f : XM_PIDIV2;

			SpawnLightFixture(CellCenter(col, row) + Vector3(0.0f, WALL_HEIGHT - 0.05f, 0.0f), yRotation);
		}
	}

	// STEP13: floating dust in each of the 4 rooms (SPEC: "reference-image
	// style floating white motes, scattered through the room"). Corridor
	// cells don't get one -- rooms only, per the request.
	SpawnDustMotes(CellCenter(2, 2)); // room A
	SpawnDustMotes(CellCenter(2, 6)); // room B
	SpawnDustMotes(CellCenter(9, 2)); // room C
	SpawnDustMotes(CellCenter(9, 6)); // room N
}
