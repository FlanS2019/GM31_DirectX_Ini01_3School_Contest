#include "main.h"
#include "Map.h"
#include "manager.h"
#include "box.h"

namespace
{
	const int COLS = 12;
	const int ROWS = 11;
	const float CELL_SIZE = 4.0f;   // shrink this later to tighten corridors/rooms
	const float WALL_HEIGHT = 3.0f;

	// '#' = wall cell, everything else (A/B/C/N/.) = open floor.
	// Letters are just labels for readability (room A, room B, room C,
	// nurse station N) -- they're all treated as plain floor.
	//
	//   A = room A        C = room C
	//   B = room B         N = nurse station
	//   the two center columns are the corridor; it runs the full length
	//   and the last row is left open in the middle -> that gap is the exit.
	const char* g_Grid[ROWS] =
	{
		"############",
		"#AAA#..#CCC#",
		"#AAA....CCC#",
		"#AAA#..#CCC#",
		"#####..#####",
		"#BBB#..#NNN#",
		"#BBB....NNN#",
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
	for (int row = 0; row < ROWS; row++)
	{
		for (int col = 0; col < COLS; col++)
		{
			if (g_Grid[row][col] != '#') continue; // only walls need geometry; Field is the floor

			Vector3 center = CellCenter(col, row);

			Box* wall = Manager::AddGameObject<Box>();
			wall->SetPosition({ center.x, WALL_HEIGHT / 2.0f, center.z });
			wall->SetScale({ CELL_SIZE / 2.0f, WALL_HEIGHT / 2.0f, CELL_SIZE / 2.0f });
		}
	}
}
