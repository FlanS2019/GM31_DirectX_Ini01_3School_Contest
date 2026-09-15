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
#include "stool.h"
#include "woodenPallet.h"
#include "crate.h"
#include "ivy.h"
#include "debris.h"
#include "stainDirt.h"
#include "stainBlood.h"
#include <cmath>
#include <vector>

namespace
{
	const int COLS = 12;
	const int ROWS = 11;
	const float CELL_SIZE = 4.0f;
	const float WALL_HEIGHT = 3.0f;
	const float CEILING_THICKNESS = 0.2f;

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

	bool IsOpenFloor(int row, int col)
	{
		if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return false;
		return g_Grid[row][col] == '.';
	}
	bool IsWallCell(int row, int col)
	{
		if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return false;
		return g_Grid[row][col] == '#';
	}

	void ThinWallAxis(float half, bool openNeg, bool openPos, float& outOffset, float& outHalf)
	{
		if (!openNeg && !openPos)
		{
			outOffset = 0.0f;
			outHalf = half;
			return;
		}

		outHalf = half * WALL_THICKNESS_SCALE;

		if (openNeg && openPos)
		{
			outOffset = 0.0f;
		}
		else if (openPos)
		{
			outOffset = -(half - outHalf);
		}
		else // openNeg
		{
			outOffset = (half - outHalf);
		}
	}

	struct WallRun
	{
		int row, c0, c1;
		float left, right;
	};

	void ZProfileKey(int row, int col, bool& outNorthOpen, bool& outSouthOpen)
	{
		outNorthOpen = !IsSolidCell(row - 1, col);
		outSouthOpen = !IsSolidCell(row + 1, col);
	}

	void XEdgesForRun(int row, int c0, int c1, float& outLeft, float& outRight)
	{
		float half = CELL_SIZE / 2.0f;
		if (c0 == c1)
		{
			Vector3 c = CellCenter(c0, row);
			float off, h;
			ThinWallAxis(half, !IsSolidCell(row, c0 - 1), !IsSolidCell(row, c0 + 1), off, h);
			outLeft = c.x + off - h;
			outRight = c.x + off + h;
			return;
		}

		Vector3 cl = CellCenter(c0, row);
		Vector3 cr = CellCenter(c1, row);
		float offL, hL, offR, hR;
		ThinWallAxis(half, !IsSolidCell(row, c0 - 1), false, offL, hL);
		ThinWallAxis(half, false, !IsSolidCell(row, c1 + 1), offR, hR);
		outLeft = cl.x + offL - hL;
		outRight = cr.x + offR + hR;
	}

	void SpawnMergedWalls()
	{
		std::vector<WallRun> rowRuns;
		for (int row = 0; row < ROWS; row++)
		{
			int col = 0;
			while (col < COLS)
			{
				if (!IsWallCell(row, col)) { col++; continue; }

				int c0 = col;
				bool prof0N, prof0S;
				ZProfileKey(row, col, prof0N, prof0S);
				int c1 = col;
				while (c1 + 1 < COLS && IsWallCell(row, c1 + 1))
				{
					bool nN, nS;
					ZProfileKey(row, c1 + 1, nN, nS);
					if (nN != prof0N || nS != prof0S) break;
					c1++;
				}

				float left, right;
				XEdgesForRun(row, c0, c1, left, right);
				rowRuns.push_back({ row, c0, c1, left, right });
				col = c1 + 1;
			}
		}

		std::vector<int> runsByRow[ROWS];
		for (size_t i = 0; i < rowRuns.size(); i++)
			runsByRow[rowRuns[i].row].push_back((int)i);

		std::vector<bool> used(rowRuns.size(), false);

		for (int row = 0; row < ROWS; row++)
		{
			for (int idx : runsByRow[row])
			{
				if (used[idx]) continue;
				used[idx] = true;
				const WallRun& r = rowRuns[idx];
				int row0 = row;
				int row1 = row;
				while (true)
				{
					int nr = row1 + 1;
					int matchIdx = -1;
					if (nr < ROWS)
					{
						for (int cand : runsByRow[nr])
						{
							const WallRun& cr = rowRuns[cand];
							if (cr.c0 == r.c0 && cr.c1 == r.c1 &&
								fabsf(cr.left - r.left) < 1e-4f && fabsf(cr.right - r.right) < 1e-4f)
							{
								matchIdx = cand;
								break;
							}
						}
					}
					if (matchIdx < 0) break;
					if (used[matchIdx]) break;
					used[matchIdx] = true;
					row1 = nr;
				}

				float half = CELL_SIZE / 2.0f;
				float top, bottom;
				if (row0 == row1)
				{
					Vector3 c = CellCenter(r.c0, row0);
					float off, h;
					ThinWallAxis(half, !IsSolidCell(row0 - 1, r.c0), !IsSolidCell(row0 + 1, r.c0), off, h);
					top = c.z + off - h;
					bottom = c.z + off + h;
				}
				else
				{
					Vector3 c0v = CellCenter(r.c0, row0);
					Vector3 c1v = CellCenter(r.c0, row1);
					float offT, hT, offB, hB;
					ThinWallAxis(half, !IsSolidCell(row0 - 1, r.c0), false, offT, hT);
					ThinWallAxis(half, false, !IsSolidCell(row1 + 1, r.c0), offB, hB);
					top = c0v.z + offT - hT;
					bottom = c1v.z + offB + hB;
				}

				float cx = (r.left + r.right) / 2.0f;
				float cz = (top + bottom) / 2.0f;
				float hx = (r.right - r.left) / 2.0f;
				float hz = (bottom - top) / 2.0f;

				Box* wall = Manager::AddGameObject<Box>();
				wall->SetPosition({ cx, WALL_HEIGHT / 2.0f, cz });
				wall->SetScale({ hx, WALL_HEIGHT / 2.0f, hz });
			}
		}
	}

	bool IsCeilingCovered(int row, int col)
	{
		(void)row;
		(void)col;
		return true;
	}

	void SpawnMergedCeiling()
	{
		struct CeilRun
		{
			int row, c0, c1;
			float left, right, top, bottom;
		};

		float half = CELL_SIZE / 2.0f;
		std::vector<CeilRun> rowRuns;
		for (int row = 0; row < ROWS; row++)
		{
			int col = 0;
			while (col < COLS)
			{
				if (!IsCeilingCovered(row, col)) { col++; continue; }
				int c0 = col;
				int c1 = col;
				while (c1 + 1 < COLS && IsCeilingCovered(row, c1 + 1)) c1++;

				Vector3 cl = CellCenter(c0, row);
				Vector3 cr = CellCenter(c1, row);
				rowRuns.push_back({ row, c0, c1, cl.x - half, cr.x + half, cl.z - half, cl.z + half });
				col = c1 + 1;
			}
		}

		std::vector<int> runsByRow[ROWS];
		for (size_t i = 0; i < rowRuns.size(); i++)
			runsByRow[rowRuns[i].row].push_back((int)i);

		std::vector<bool> used(rowRuns.size(), false);

		for (int row = 0; row < ROWS; row++)
		{
			for (int idx : runsByRow[row])
			{
				if (used[idx]) continue;
				used[idx] = true;
				const CeilRun& r = rowRuns[idx];
				int row1 = row;
				float bottom = r.bottom;
				while (true)
				{
					int nr = row1 + 1;
					int matchIdx = -1;
					if (nr < ROWS)
					{
						for (int cand : runsByRow[nr])
						{
							const CeilRun& cr = rowRuns[cand];
							if (cr.c0 == r.c0 && cr.c1 == r.c1 &&
								fabsf(cr.left - r.left) < 1e-4f && fabsf(cr.right - r.right) < 1e-4f)
							{
								matchIdx = cand;
								break;
							}
						}
					}
					if (matchIdx < 0) break;
					if (used[matchIdx]) break;
					used[matchIdx] = true;
					bottom = rowRuns[matchIdx].bottom;
					row1 = nr;
				}

				float cx = (r.left + r.right) / 2.0f;
				float cz = (r.top + bottom) / 2.0f;
				float hx = (r.right - r.left) / 2.0f;
				float hz = (bottom - r.top) / 2.0f;

				Box* ceiling = Manager::AddGameObject<Box>();
				ceiling->SetPosition({ cx, WALL_HEIGHT + CEILING_THICKNESS / 2.0f, cz });
				ceiling->SetScale({ hx, CEILING_THICKNESS / 2.0f, hz });
				ceiling->SetBlocking(false);
			}
		}
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

	void SpawnStool(const Vector3& position, float yRotation)
	{
		Stool* stool = Manager::AddGameObject<Stool>();
		stool->SetPosition(position);
		stool->SetRotation({ 0.0f, yRotation, 0.0f });
	}

	void SpawnFallenStool(const Vector3& position, float yRotation)
	{
		const float kFallenLift = 0.21f;

		Stool* stool = Manager::AddGameObject<Stool>();
		stool->SetPosition({ position.x, position.y + kFallenLift, position.z });
		stool->SetRotation({ XM_PIDIV2, yRotation, 0.0f });
	}

	void SpawnLeaningPallet(const Vector3& floorPosition, float wallYRotation)
	{
		const float kLeanFromVertical = 0.25f;
		const float kStandPitch = XM_PIDIV2 - kLeanFromVertical;
		const float kStandLift = 0.48f;

		WoodenPallet* pallet = Manager::AddGameObject<WoodenPallet>();
		pallet->SetPosition({ floorPosition.x, floorPosition.y + kStandLift, floorPosition.z });
		pallet->SetRotation({ kStandPitch, wallYRotation, 0.0f });
	}

	void SpawnFlatPallet(const Vector3& floorPosition, float yRotation)
	{
		WoodenPallet* pallet = Manager::AddGameObject<WoodenPallet>();
		pallet->SetPosition(floorPosition);
		pallet->SetRotation({ 0.0f, yRotation, 0.0f });
	}

	Door* SpawnDoorway(int row, int col, bool widthIsZ)
	{
		Vector3 center = CellCenter(col, row);
		const float kCellHalf = CELL_SIZE / 2.0f;
		const float kFrameHalfWidth = 0.537f; 
		const float kDoorYaw = widthIsZ ? XM_PIDIV2 : 0.0f;
		const float kFlankHalfWidth = (kCellHalf - kFrameHalfWidth) / 2.0f;
		const float kFlankCenterOffset = (kFrameHalfWidth + kCellHalf) / 2.0f;

		float thicknessOffset, thicknessHalf;
		if (widthIsZ)
			ThinWallAxis(kCellHalf, !IsSolidCell(row, col - 1), !IsSolidCell(row, col + 1), thicknessOffset, thicknessHalf);
		else
			ThinWallAxis(kCellHalf, !IsSolidCell(row - 1, col), !IsSolidCell(row + 1, col), thicknessOffset, thicknessHalf);

		Vector3 doorPos = center;
		if (widthIsZ)
			doorPos.x += thicknessOffset;
		else
			doorPos.z += thicknessOffset;

		Door* door = Manager::AddGameObject<Door>();
		door->SetPosition({ doorPos.x, WALL_HEIGHT / 2.0f, doorPos.z });
		door->SetScale({ kCellHalf, WALL_HEIGHT / 2.0f, kCellHalf }); 
		door->SetRotation({ 0.0f, kDoorYaw, 0.0f });

		float sides[2] = { -1.0f, 1.0f };
		for (int i = 0; i < 2; i++)
		{
			Box* flank = Manager::AddGameObject<Box>();
			float widthOffset = sides[i] * kFlankCenterOffset;

			if (widthIsZ)
			{
				flank->SetPosition({ center.x + thicknessOffset, WALL_HEIGHT / 2.0f, center.z + widthOffset });
				flank->SetScale({ thicknessHalf, WALL_HEIGHT / 2.0f, kFlankHalfWidth });
			}
			else
			{
				flank->SetPosition({ center.x + widthOffset, WALL_HEIGHT / 2.0f, center.z + thicknessOffset });
				flank->SetScale({ kFlankHalfWidth, WALL_HEIGHT / 2.0f, thicknessHalf });
			}
		}

		const float kFrameTopY = 2.175f; 
		if (kFrameTopY < WALL_HEIGHT)
		{
			float lintelHalfHeight = (WALL_HEIGHT - kFrameTopY) / 2.0f;
			float lintelCenterY = kFrameTopY + lintelHalfHeight;

			Box* lintel = Manager::AddGameObject<Box>();
			if (widthIsZ)
			{
				lintel->SetPosition({ center.x + thicknessOffset, lintelCenterY, center.z });
				lintel->SetScale({ thicknessHalf, lintelHalfHeight, kFrameHalfWidth });
			}
			else
			{
				lintel->SetPosition({ center.x, lintelCenterY, center.z + thicknessOffset });
				lintel->SetScale({ kFrameHalfWidth, lintelHalfHeight, thicknessHalf });
			}
		}

		return door;
	}

	void SpawnCrate(const Vector3& position, float yRotation)
	{
		Crate* crate = Manager::AddGameObject<Crate>();
		crate->SetPosition(position);
		crate->SetRotation({ 0.0f, yRotation, 0.0f });
	}

	void SpawnIvy(const Vector3& position, float wallYRotation)
	{
		Ivy* ivy = Manager::AddGameObject<Ivy>();
		ivy->SetPosition({ position.x, WALL_HEIGHT, position.z });
		ivy->SetRotation({ 0.0f, wallYRotation, 0.0f });
	}

	void SpawnDebris(const Vector3& position, float yRotation, float scale = 1.0f, float tilt = 0.0f)
	{
		Debris* debris = Manager::AddGameObject<Debris>();
		debris->SetPosition(position);
		debris->SetRotation({ tilt, yRotation, 0.0f });
		debris->SetScale({ scale, scale, scale });
	}

	void SpawnDirtStain(const Vector3& position, float yRotation, float scale = 1.0f)
	{
		StainDirt* stain = Manager::AddGameObject<StainDirt>();
		stain->SetPosition(position);
		stain->SetRotation({ 0.0f, yRotation, 0.0f });
		stain->SetScale({ scale, 1.0f, scale });
	}

	void SpawnBloodStain(const Vector3& position, float yRotation, float scale = 1.0f)
	{
		StainBlood* stain = Manager::AddGameObject<StainBlood>();
		stain->SetPosition(position);
		stain->SetRotation({ 0.0f, yRotation, 0.0f });
		stain->SetScale({ scale, 1.0f, scale });
	}
}

void Map::Init()
{
	Door* gimmickDoor = nullptr;

	SpawnMergedWalls();

	for (int row = 0; row < ROWS; row++)
	{
		for (int col = 0; col < COLS; col++)
		{
			if (g_Grid[row][col] == 'G')
			{
				gimmickDoor = SpawnDoorway(row, col, true);
				gimmickDoor->SetSwitchOnly(true);
			}
		}
	}

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
				break;
			}
			case 'D':
			{
				Door* door = SpawnDoorway(row, col, true);
				door->SetRequiredKey(0); // matches the 'K' key below
				break;
			}
			case 'G':
			{
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
				Vector3 westWallCenter = CellCenter(col - 1, row);
				float wallOff, wallHalf;
				ThinWallAxis(CELL_SIZE / 2.0f, !IsSolidCell(row, col - 2), !IsSolidCell(row, col), wallOff, wallHalf);
				float wallFaceX = westWallCenter.x + wallOff + wallHalf;
				const float kPlateHalfThickness = 0.0225f; // Switch_Lever.obj PlateMetal Z half-extent (0.045) * m_Scale.z (0.5)
				const float kMountHeight = 1.2f;
				sw->SetPosition({ wallFaceX + kPlateHalfThickness, kMountHeight, center.z });
				sw->SetRotation({ 0.0f, XM_PIDIV2, 0.0f });
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
				item->SetDisplayName("色あせた手紙");
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
				Door* door = SpawnDoorway(row, col, false);
				door->SetRequiredKey(4);
				door->SetIsExit(true);
				break;
			}
			default:
				break; 
			}
		}
	}

	SpawnMergedCeiling();

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

	SpawnStool(CellCenter(1, 1) + Vector3(0.6f, 0.0f, -0.5f), 0.9f);
	SpawnStool(CellCenter(3, 3) + Vector3(-0.4f, 0.0f, 0.7f), 2.5f);
	SpawnFallenStool(CellCenter(3, 1) + Vector3(-0.5f, 0.0f, 0.4f), 1.0f);
	SpawnLeaningPallet(CellCenter(1, 2) + Vector3(-4.73f, 0.0f, 0.2f), XM_PIDIV2);
	SpawnFlatPallet(CellCenter(2, 3) + Vector3(0.3f, 0.0f, -0.6f), 0.6f);

	SpawnStool(CellCenter(3, 5) + Vector3(0.5f, 0.0f, -0.6f), 1.2f);
	SpawnStool(CellCenter(2, 7) + Vector3(-0.3f, 0.0f, 0.5f), 3.0f);
	SpawnFallenStool(CellCenter(1, 6) + Vector3(0.4f, 0.0f, -0.3f), 2.2f);
	SpawnLeaningPallet(CellCenter(2, 7) + Vector3(0.3f, 0.0f, 4.57f), 0.0f);
	SpawnFlatPallet(CellCenter(3, 6) + Vector3(-0.4f, 0.0f, 0.5f), 1.8f);

	SpawnStool(CellCenter(10, 1) + Vector3(-0.5f, 0.0f, 0.6f), 1.7f);
	SpawnStool(CellCenter(8, 3) + Vector3(0.6f, 0.0f, -0.4f), 0.3f);
	SpawnFallenStool(CellCenter(9, 3) + Vector3(0.3f, 0.0f, 0.5f), 0.4f);
	SpawnLeaningPallet(CellCenter(10, 2) + Vector3(4.57f, 0.0f, -0.2f), XM_PIDIV2);
	SpawnFlatPallet(CellCenter(8, 1) + Vector3(-0.4f, 0.0f, -0.3f), 2.6f);

	SpawnStool(CellCenter(10, 6) + Vector3(0.4f, 0.0f, -0.6f), 2.0f);
	SpawnStool(CellCenter(8, 7) + Vector3(-0.5f, 0.0f, 0.5f), 0.8f);
	SpawnFallenStool(CellCenter(10, 7) + Vector3(-0.3f, 0.0f, -0.5f), 3.0f);
	SpawnLeaningPallet(CellCenter(9, 7) + Vector3(0.2f, 0.0f, 4.57f), 0.0f);
	SpawnFlatPallet(CellCenter(8, 6) + Vector3(0.4f, 0.0f, 0.3f), 1.2f);

	SpawnStool(CellCenter(5, 2) + Vector3(0.3f, 0.0f, -0.4f), 1.4f);
	SpawnStool(CellCenter(6, 6) + Vector3(-0.3f, 0.0f, 0.4f), 3.6f);

	SpawnCrate(CellCenter(1, 5) + Vector3(-0.3f, 0.0f, 0.4f), 0.7f);   // room B
	SpawnCrate(CellCenter(9, 2) + Vector3(-0.2f, 0.0f, -0.3f), 2.1f);  // room C
	SpawnIvy(CellCenter(9, 5) + Vector3(0.0f, 0.0f, -(CELL_SIZE / 2.0f - 0.15f)), 0.0f); // room N 北壁

	SpawnIvy(CellCenter(2, 1) + Vector3(0.0f, 0.0f, -(CELL_SIZE / 2.0f - 0.15f)), 0.0f); // room A 北壁
	SpawnIvy(CellCenter(9, 1) + Vector3(0.0f, 0.0f, -(CELL_SIZE / 2.0f - 0.15f)), 0.0f); // room C 北壁
	SpawnIvy(CellCenter(2, 5) + Vector3(0.0f, 0.0f, -(CELL_SIZE / 2.0f - 0.15f)), 0.0f); // room B 北壁
	SpawnIvy(CellCenter(8, 5) + Vector3(0.3f, 0.0f, -(CELL_SIZE / 2.0f - 0.15f)), 0.0f); // room N 北壁(もう1本、密度アップ)

	// room A
	SpawnDirtStain(CellCenter(2, 1) + Vector3(0.25f, 0.0f, 0.3f), 0.4f, 1.1f);
	SpawnDebris(CellCenter(1, 3) + Vector3(0.3f, 0.0f, -0.2f), 1.0f, 0.9f);

	// room B
	SpawnDebris(CellCenter(1, 5) + Vector3(0.4f, 0.0f, 0.3f), 2.4f, 1.0f);
	SpawnBloodStain(CellCenter(3, 7) + Vector3(-0.2f, 0.0f, 0.3f), 1.2f, 0.9f);

	// room C
	SpawnDirtStain(CellCenter(9, 3) + Vector3(-0.3f, 0.0f, -0.2f), 2.0f, 0.95f);
	SpawnDebris(CellCenter(10, 3) + Vector3(0.2f, 0.0f, 0.4f), 0.3f, 1.1f);

	SpawnDirtStain(CellCenter(10, 5) + Vector3(0.3f, 0.0f, -0.3f), 0.7f, 1.0f);
	SpawnBloodStain(CellCenter(9, 7) + Vector3(-0.25f, 0.0f, 0.25f), 3.4f, 1.2f);
	SpawnDebris(CellCenter(9, 6) + Vector3(-0.3f, 0.0f, 0.35f), 1.8f, 0.85f);

	// 中央廊下にも少し
	SpawnDirtStain(CellCenter(5, 4) + Vector3(0.2f, 0.0f, 0.0f), 1.5f, 1.0f);
	SpawnDebris(CellCenter(6, 8) + Vector3(-0.2f, 0.0f, 0.3f), 2.7f, 1.0f);
	SpawnDirtStain(CellCenter(6, 2) + Vector3(-0.15f, 0.0f, 0.2f), 0.9f, 0.85f);

	SpawnBloodStain(CellCenter(3, 2) + Vector3(0.3f, 0.0f, -0.2f), 1.7f, 0.9f);   // room A
	SpawnBloodStain(CellCenter(6, 7) + Vector3(0.15f, 0.0f, -0.2f), 0.6f, 0.9f);  // 廊下(出口へ向かう血の跡 1/4)
	SpawnBloodStain(CellCenter(5, 8) + Vector3(-0.25f, 0.0f, 0.15f), 2.1f, 1.0f); // 廊下(2/4)
	SpawnBloodStain(CellCenter(6, 9) + Vector3(0.1f, 0.0f, -0.3f), 1.0f, 0.95f);  // 廊下(3/4)
	SpawnBloodStain(CellCenter(5, 9) + Vector3(-0.1f, 0.0f, 0.25f), 3.0f, 1.3f);  // 廊下(4/4、出口直前で一番大きく)

	SpawnDebris(CellCenter(5, 1) + Vector3(0.3f, 0.0f, -0.2f), 1.6f, 0.9f);
	SpawnDirtStain(CellCenter(6, 1) + Vector3(-0.2f, 0.0f, 0.3f), 0.5f, 1.0f);
	SpawnCrate(CellCenter(4, 2) + Vector3(0.0f, 0.0f, 0.4f), 3.3f);           // 部屋Cへのドア手前、廈下側に見張り番したがりのように
	SpawnDebris(CellCenter(5, 3) + Vector3(-0.3f, 0.0f, 0.2f), 2.2f, 1.0f);
	SpawnBloodStain(CellCenter(6, 3) + Vector3(0.2f, 0.0f, -0.3f), 1.9f, 0.85f);
	SpawnDirtStain(CellCenter(5, 5) + Vector3(0.3f, 0.0f, 0.15f), 2.6f, 1.05f);
	SpawnDebris(CellCenter(6, 5) + Vector3(-0.25f, 0.0f, -0.3f), 0.7f, 0.95f);
	SpawnFallenStool(CellCenter(7, 6) + Vector3(0.0f, 0.0f, 0.3f), 1.1f);      // ギミックドアG(col4,row6)のすぐ横、幅の広いrow6の三マス目
}
