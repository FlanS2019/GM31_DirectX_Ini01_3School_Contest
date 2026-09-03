#include "main.h"
#include "Map.h"
#include "manager.h"
#include "box.h"
#include "door.h"
#include "key.h"
#include "switch.h"

namespace
{
	const int COLS = 12;
	const int ROWS = 11;
	const float CELL_SIZE = 4.0f;   // shrink this later to tighten corridors/rooms
	const float WALL_HEIGHT = 3.0f;

	// '#' = wall cell. '.' and the room-label letters (A/B/C/N) are all
	// plain open floor -- the letters are just for readability.
	// STEP2 additions, placed on cells that used to be plain floor:
	//   K = key pickup (id 0)
	//   D = locked door, needs key id 0 (gates room C off the corridor)
	//   G = gimmick door, opens only from its linked Switch (gates room B)
	//   X = the switch that opens the 'G' door (sits inside the nurse
	//       station, N, so reaching it means passing through N first)
	//
	//   A = room A        C = room C (behind the D door)
	//   B = room B (behind the G door)   N = nurse station
	//   the two center columns are the corridor; it runs the full length
	//   and the last row is left open in the middle -> that gap is the exit.
	const char* g_Grid[ROWS] =
	{
		"############",
		"#AAA#..#CCC#",
		"#AKA...DCCC#",
		"#AAA#..#CCC#",
		"#####..#####",
		"#BBB#..#NNN#",
		"#BBBG...NXN#",
		"#BBB#..#NNN#",
		"#####..#####",
		"#####..#####",
		"#####..#####",
	};

	Vector3 CellCenter(int col, int row)
	{
		float x = (col - COLS / 2.0f + 0.5f) * CELL_SIZE;
		float z = (row - ROWS / 2.0f + 0.5f) * CELL_SIZE;
		return Vector3(x, 0.0f, z);
	}
}

void Map::Init()
{
	// The 'G' door and the 'X' switch that opens it are wired together by
	// pointer once both exist. The grid is scanned in a single row-major
	// pass, and 'G' (row6 col4) comes before 'X' (row6 col9) in that scan,
	// so by the time 'X' is reached the Door* below is already set. If a
	// layout ever puts a switch before its target door, this simple
	// single-door version would need to become a lookup by id instead.
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
				wall->SetPosition({ center.x, WALL_HEIGHT / 2.0f, center.z });
				wall->SetScale({ CELL_SIZE / 2.0f, WALL_HEIGHT / 2.0f, CELL_SIZE / 2.0f });
				break;
			}
			case 'D':
			{
				Door* door = Manager::AddGameObject<Door>();
				door->SetPosition({ center.x, WALL_HEIGHT / 2.0f, center.z });
				door->SetScale({ CELL_SIZE / 2.0f, WALL_HEIGHT / 2.0f, CELL_SIZE / 2.0f });
				door->SetSlideDirection(Door::SlideDirection::NegZ);
				door->SetRequiredKey(0); // matches the 'K' key below
				break;
			}
			case 'G':
			{
				Door* door = Manager::AddGameObject<Door>();
				door->SetPosition({ center.x, WALL_HEIGHT / 2.0f, center.z });
				door->SetScale({ CELL_SIZE / 2.0f, WALL_HEIGHT / 2.0f, CELL_SIZE / 2.0f });
				door->SetSlideDirection(Door::SlideDirection::NegZ); // D”à‚Æ“¯‚¶‚­‰¡ƒXƒ‰ƒCƒh
				// no SetRequiredKey() -- only the 'X' switch can open this one
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
				sw->SetPosition({ center.x, 0.0f, center.z });
				sw->SetTargetDoor(gimmickDoor);
				break;
			}
			default:
				break; // plain floor; Field is the floor mesh
			}
		}
	}
}
