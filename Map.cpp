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

	const float WALL_THICKNESS_SCALE = 0.3f;

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

	void SpawnLightFixture(const Vector3& position, float yRotation, bool isLit, bool flicker = false)
	{
		const float kFaceDownPitch = XM_PIDIV2;
		const float kTubeDrop = 0.12f;
		const float kTubeSideOffset = 0.05f;

		LightCase* lightCase = Manager::AddGameObject<LightCase>();
		lightCase->SetPosition(position);
		lightCase->SetRotation({ kFaceDownPitch, yRotation, 0.0f });

		if (!isLit)
			return;

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
			lightTube->SetFlicker(flicker);
		}
	}

	// 管が半分外れて斜めに垂れ下がってる状態。管1本だけ、余分に傾けて低め
	// に垂らしてて、常にチカチカさせる(外れかけの管が安定して光り続ける
	// のは不自然なので)。1本だけ完全に「光らない」固定にするには非発光の
	// 別素材が要る(共有マテリアルの都合)ので、それは床落ち管をやる時に
	// 改めて検討する。
	void SpawnDisplacedTube(const Vector3& position, float yRotation)
	{
		const float kFaceDownPitch = XM_PIDIV2;
		const float kTubeDrop = 0.12f;
		const float kExtraDrop = 0.15f;
		const float kExtraTilt = 0.5f;

		LightCase* lightCase = Manager::AddGameObject<LightCase>();
		lightCase->SetPosition(position);
		lightCase->SetRotation({ kFaceDownPitch, yRotation, 0.0f });

		LightTube* lightTube = Manager::AddGameObject<LightTube>();
		lightTube->SetPosition({ position.x, position.y - kTubeDrop - kExtraDrop, position.z });
		lightTube->SetRotation({ kFaceDownPitch + kExtraTilt, yRotation, kExtraTilt });
		lightTube->SetFlicker(true);
	}

	// 部屋ごとの配置パターン: 3x3マスの部屋に3個(左上・右上・下中央)、
// 2点灯+1消灯。(colBase, rowBase)はその部屋の左上マスの座標。
	void SpawnRoomLights(int colBase, int rowBase)
	{
		SpawnLightFixture(CellCenter(colBase, rowBase) + Vector3(0.0f, WALL_HEIGHT - 0.05f, 0.0f), XM_PIDIV2, true, true);  // チカチカ
		SpawnLightFixture(CellCenter(colBase + 2, rowBase) + Vector3(0.0f, WALL_HEIGHT - 0.05f, 0.0f), XM_PIDIV2, true, false); // 安定点灯
		SpawnLightFixture(CellCenter(colBase + 1, rowBase + 2) + Vector3(0.0f, WALL_HEIGHT - 0.05f, 0.0f), XM_PIDIV2, false);        // 消灯
	}

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
	SpawnRoomLights(1, 1); // room A
	SpawnRoomLights(1, 5); // room B
	SpawnRoomLights(8, 1); // room C
	SpawnRoomLights(8, 5); // room N

	for (int row = 0; row < ROWS; row++)
	{
		for (int col = 0; col < COLS; col++)
		{
			if (!IsOpenFloor(row, col)) continue;
			if ((row + col) % 2 != 0) continue;

			bool northSouthOpen = IsOpenFloor(row - 1, col) || IsOpenFloor(row + 1, col);
			bool eastWestOpen = IsOpenFloor(row, col - 1) || IsOpenFloor(row, col + 1);
			float yRotation = (eastWestOpen && !northSouthOpen) ? 0.0f : XM_PIDIV2;
			Vector3 fixturePos = CellCenter(col, row) + Vector3(0.0f, WALL_HEIGHT - 0.05f, 0.0f);

			int pattern = (row * COLS + col) % 3;
			if (pattern == 0)
				SpawnLightFixture(fixturePos, yRotation, false);       // 消灯
			else if (pattern == 1)
				SpawnDisplacedTube(fixturePos, yRotation);             // 外れかけ
			else
				SpawnLightFixture(fixturePos, yRotation, true);        // 通常点灯
		}
	}

	SpawnDustMotes(CellCenter(2, 2)); // room A
	SpawnDustMotes(CellCenter(2, 6)); // room B
	SpawnDustMotes(CellCenter(9, 2)); // room C
	SpawnDustMotes(CellCenter(9, 6)); // room N
}
