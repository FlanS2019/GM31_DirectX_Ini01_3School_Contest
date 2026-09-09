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
#include <cmath>

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

	void PlaceWall(Box* wall, const Vector3& center, int row, int col)
	{
		float offsetX, halfX, offsetZ, halfZ;
		ThinWallAxis(CELL_SIZE / 2.0f, !IsSolidCell(row, col - 1), !IsSolidCell(row, col + 1), offsetX, halfX);
		ThinWallAxis(CELL_SIZE / 2.0f, !IsSolidCell(row - 1, col), !IsSolidCell(row + 1, col), offsetZ, halfZ);

		wall->SetPosition({ center.x + offsetX, WALL_HEIGHT / 2.0f, center.z + offsetZ });
		wall->SetScale({ halfX, WALL_HEIGHT / 2.0f, halfZ });
	}

	void SpawnLightFixture(const Vector3& position, float yRotation, bool isLit)
	{
		const float kFaceDownPitch = XM_PIDIV2;

		// kTubeDrop = 0.12 -- confirmed working in-game (measured from
		// LightCase's own vertex bounds; its local origin sits at the
		// housing's back, not its center).
		const float kTubeDrop = 0.12f;

		// Two tubes side by side, offset perpendicular to the tube's own
		// length. That direction is local Y before rotation, which becomes
		// world Z at yRotation=0 or world X at yRotation=XM_PIDIV2 -- see
		// sideOffset below. Kept well inside the case's own local-Y half
		// width (~0.138) so both tubes stay under the housing.
		const float kTubeSideOffset = 0.05f;

		LightCase* lightCase = Manager::AddGameObject<LightCase>();
		lightCase->SetPosition(position);
		lightCase->SetRotation({ kFaceDownPitch, yRotation, 0.0f });

		if (!isLit)
			return; // dead fixture -- empty housing only, no tube inside

		Vector3 sideOffset(kTubeSideOffset * sinf(yRotation), 0.0f, kTubeSideOffset * cosf(yRotation));

		for (int i = 0; i < 2; i++)
		{
			float side = (i == 0) ? -1.0f : 1.0f;

			LightTube* lightTube = Manager::AddGameObject<LightTube>();
			lightTube->SetPosition({
				position.x + side * sideOffset.x,
				position.y - kTubeDrop,
				position.z + side * sideOffset.z });
			lightTube->SetRotation({ kFaceDownPitch, yRotation, 0.0f });
		}
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
				item->SetDisplayName("古い写真");
				break;
			}
			case 'R':
			{
				Item* item = Manager::AddGameObject<Item>();
				item->SetPosition({ center.x, item->GetPosition().y, center.z });
				item->SetItemId(2);
				item->SetDisplayName("色あせた手紙"); // STEP13: was a hospital "diagnosis record" -- swapped for something a ruins theme actually fits
				break;
			}
			case 'M':
			{
				Item* item = Manager::AddGameObject<Item>();
				item->SetPosition({ center.x, item->GetPosition().y, center.z });
				item->SetItemId(3);
				item->SetDisplayName("金属部品");
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

	// loop is the one place to fix it.
	// 各部屋1個ずつ(SpawnDustMotesと同じ4部屋の中心を再利用)。isLitで
	// 点灯/消灯を部屋ごとに指定 -- 自由に true/false を入れ替えてOK。
	SpawnLightFixture(CellCenter(2, 2) + Vector3(0.0f, WALL_HEIGHT - 0.05f, 0.0f), XM_PIDIV2, true);  // room A
	SpawnLightFixture(CellCenter(2, 6) + Vector3(0.0f, WALL_HEIGHT - 0.05f, 0.0f), XM_PIDIV2, false); // room B (消灯)
	SpawnLightFixture(CellCenter(9, 2) + Vector3(0.0f, WALL_HEIGHT - 0.05f, 0.0f), XM_PIDIV2, true);  // room C
	SpawnLightFixture(CellCenter(9, 6) + Vector3(0.0f, WALL_HEIGHT - 0.05f, 0.0f), XM_PIDIV2, false); // room N (消灯)

	SpawnDustMotes(CellCenter(2, 2)); // room A
	SpawnDustMotes(CellCenter(2, 6)); // room B
	SpawnDustMotes(CellCenter(9, 2)); // room C
	SpawnDustMotes(CellCenter(9, 6)); // room N
}
