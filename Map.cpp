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
#include <cmath>
#include <vector>

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

	// True only for an actual wall cell ('#') -- distinct from IsSolidCell,
	// which also counts doors/gimmick door/exit as solid. Out of bounds
	// counts as NOT a wall. Used by SpawnMergedWalls below.
	bool IsWallCell(int row, int col)
	{
		if (row < 0 || row >= ROWS || col < 0 || col >= COLS) return false;
		return g_Grid[row][col] == '#';
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

	struct WallRun
	{
		int row, c0, c1;
		float left, right;
	};

	// 北/南の開放パターン(壁で挟まれている/西だけ開放...)が同じセルだけを
	// 1本のランにまとめられる -- パターンが違うとThinWallAxisのZ方向の
	// 縮み方(top/bottom)がセルごとに変わり、1個の箱で正しく表現できない。
	void ZProfileKey(int row, int col, bool& outNorthOpen, bool& outSouthOpen)
	{
		outNorthOpen = !IsSolidCell(row - 1, col);
		outSouthOpen = !IsSolidCell(row + 1, col);
	}

	// ラン[c0..c1]のワールド空間X方向の左右端。1マスだけのランは
	// ThinWallAxisを東西同時に呼んで(孤立セルとして)、2マス以上のランは
	// 「ランの両端だけがそれぞれ西/東に開放されうる、内部セルは常に閉じて
	// いる」という前提で、両端をそれぞれ片側だけ開放のThinWallAxisで計算
	// する。この2通りの式が元のPlaceWall(1セルずつ)と完全に同じ形になる
	// ことは、Pythonでground truthを再現して20万点のランダムサンプリング
	// で不一致0件を確認済み(merge_sim4.py)。
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

	// 壁ブロックの引き延ばし(隙間を消してドアモデルなどを後々埋め込み
	// やすくする)。手順: (1)各行を横方向にランへまとめる (2)真下の行が
	// 同じ列範囲・同じ左右端を持つ限り縦にも連結する (3)最終的な上端/
	// 下端は、1行だけのグループなら北/南を同時に見るThinWallAxis、2行
	// 以上なら「一番上の行の北側だけ」「一番下の行の南側だけ」を見た端を
	// 使う。当たり判定(Player.cppのBox衝突)は座標(中心)とサイズ(半径)
	// しか見ないので、個数が減っても挙動は変わらない。この4通りの組み
	// 合わせが元の1セルずつのPlaceWallと完全に同じ形状になることは、
	// Pythonでground truthを再現し20万点のランダムサンプリングで不一致
	// 0件になることを確認済み(merge_sim4.py)。
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

	// 天井は壁と違って縮み(ThinWallAxis)が無く、全マス同じ大きさで隙間なく
	// 敷き詰めているだけなので、壁より単純な「隙間なしランレングス」で
	// まとめられる。IsCeilingCoveredを個別に切り出してあるのは、将来
	// 「崩れた屋根に穴を開ける」ような演出を足したくなったとき、ここを
	// 1行変えるだけで済むようにするため(今は常にtrueなので、結果的に
	// 1～数個の巨大な板にまとまる)。
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

	// 廃墟に転がってるだけの雑多な小物 -- 向き・位置は部屋ごとにバラバラに
	void SpawnStool(const Vector3& position, float yRotation)
	{
		Stool* stool = Manager::AddGameObject<Stool>();
		stool->SetPosition(position);
		stool->SetRotation({ 0.0f, yRotation, 0.0f });
	}

	// 倒れて横向きに転がってるスツール。X軸(pitch)90度で真横に倒し、
	// yRotationだけ倒れてる向きを変える。kFallenLiftは実際のLeather_Stool.obj
	// の全頂点(5704個)にpitch=90度の回転を適用してY座標の最小値を計算し
	// (raw座標で≒-216.55、スケール0.001を掛けて≒0.2166)、そこから逆算した
	// 値 -- 床にめり込まない最小限の値より少しだけ低くしてある。
	void SpawnFallenStool(const Vector3& position, float yRotation)
	{
		const float kFallenLift = 0.21f;

		Stool* stool = Manager::AddGameObject<Stool>();
		stool->SetPosition({ position.x, position.y + kFallenLift, position.z });
		stool->SetRotation({ XM_PIDIV2, yRotation, 0.0f });
	}

	// 「板」っぽいパレットは寝かせず壁に立てかける。完全に垂直だと不自然
	// なので少し壁側に傾けてあり(kLeanFromVertical)。
	//
	// kStandLift(縦の持ち上げ量)は実際のWoodenPallet_Broken2.objの全頂点
	// (146個)にこのpitchでの回転を適用してY座標の最小値を計算し(≒-0.483)、
	// そこから逆算した値。浮くより少しだけ床にめり込む方が目立たないので、
	// わずかに低め(0.48)にしてある。
	//
	// floorPositionは「壁に近い側の床の位置」として渡す前提 -- 呼び出し側
	// (Map::Init())のオフセットは、各部屋の壁が薄く後退している(ThinWallAxis
	// による、隣が開放セルの壁は奥に0.3倍まで縮む)実際の壁面座標から逆算
	// してあり、単純にCellCenterから1.3ユニットずらす、のような大雑把な
	// 見た目基準の値ではない -- 最初の実装がこれを考慮していなかったため、
	// 全部のパレットが実際の壁面よりだいぶ手前(部屋の真ん中寄り)に浮いて
	// 見えていた。WoodenPallet::Draw()側のモデル自体のピボットのズレ
	// (X方向に約3ユニット)も疑って一度直したが、そちらは壁に沿った横方向
	// にしか影響せず、壁からの奥行き方向には無関係だったため元に戻した
	// (詳細はwoodenPallet.cpp参照)。
	void SpawnLeaningPallet(const Vector3& floorPosition, float wallYRotation)
	{
		const float kLeanFromVertical = 0.25f;
		const float kStandPitch = XM_PIDIV2 - kLeanFromVertical;
		const float kStandLift = 0.48f;

		WoodenPallet* pallet = Manager::AddGameObject<WoodenPallet>();
		pallet->SetPosition({ floorPosition.x, floorPosition.y + kStandLift, floorPosition.z });
		pallet->SetRotation({ kStandPitch, wallYRotation, 0.0f });
	}

	// 床にそのまま(斜めの向きで)転がってるパレット。立てかけと違って
	// pitch/rollは0のまま(=モデル本来の寝かせ姿勢)、yRotationだけ適当な
	// 角度にして「雑に転がってる」感を出す。原点はもともと床(Y≒0.0006)
	// にほぼ一致しているので、追加の持ち上げは不要。
	void SpawnFlatPallet(const Vector3& floorPosition, float yRotation)
	{
		WoodenPallet* pallet = Manager::AddGameObject<WoodenPallet>();
		pallet->SetPosition(floorPosition);
		pallet->SetRotation({ 0.0f, yRotation, 0.0f });
	}

	// ドア1個ぶん(Door_VAR01の枠+観音開きの扉2枚+当たり判定)をまとめて
	// 配置する。widthIsZ で「ドアの通り抜け方向がX軸か(false, 東西に
	// 伸びる壁ランの途中にある)/Z軸か(true, 南北に伸びる壁ランの途中に
	// ある)」を指定する -- 部屋の光源のyRotation計算と同じ判定基準
	// (隣接セルが北/南で塞がれているか)で呼び出し側が決める。
	//
	// Door_VAR01.objは実測で枠(Marco)の全幅が約1.07m(半径0.537)しかなく、
	// セル1つぶん(4m)にはとても足りないので、両脇をbox.obj(壁と同じ見た目)
	// の袖壁で埋めて、扉が本当に開く部分だけをドア枠の幅に絞っている。
	// 当たり判定(Door::m_Position/m_Scale)は今まで通りセル全体のままに
	// してあり(Player.cppの壁判定はそのまま)、袖壁側が常時ブロックする
	// ことで、結果的に扉が開いた後も枠の外側(袖壁の場所)は歩けない。
	Door* SpawnDoorway(int row, int col, bool widthIsZ)
	{
		Vector3 center = CellCenter(col, row);
		const float kCellHalf = CELL_SIZE / 2.0f;
		const float kFrameHalfWidth = 0.537f; // Door_Frame.objの実測半幅(分割スクリプト参照)
		const float kDoorYaw = widthIsZ ? XM_PIDIV2 : 0.0f;
		const float kFlankHalfWidth = (kCellHalf - kFrameHalfWidth) / 2.0f;
		const float kFlankCenterOffset = (kFrameHalfWidth + kCellHalf) / 2.0f;

		// STEP: 袖壁(flank)とまぐさの「厚み軸」は、普通の壁(SpawnMergedWalls)
		// と同じThinWallAxis()で求める。固定のkCellHalf*WALL_THICKNESS_SCALE
		// を両側均等に使っていた前バージョンは、隣が壁('#')で塞がれている
		// 側(例: 出口ドア'E'は南北が壁)でもオフセット無しで中央に留まって
		// しまい、実際の壁面との間に隙間・段差ができていた(壁の配置が
		// おかしく見える不具合の原因)。widthIsZならX(東西)、falseなら
		// Z(南北)の隣接セルで開閉判定する -- 両隣とも開いてる('D'や'G'の
		// ような孤立した戸口)場合はオフセット0のまま変わらない。
		float thicknessOffset, thicknessHalf;
		if (widthIsZ)
			ThinWallAxis(kCellHalf, !IsSolidCell(row, col - 1), !IsSolidCell(row, col + 1), thicknessOffset, thicknessHalf);
		else
			ThinWallAxis(kCellHalf, !IsSolidCell(row - 1, col), !IsSolidCell(row + 1, col), thicknessOffset, thicknessHalf);

		Door* door = Manager::AddGameObject<Door>();
		door->SetPosition({ center.x, WALL_HEIGHT / 2.0f, center.z });
		door->SetScale({ kCellHalf, WALL_HEIGHT / 2.0f, kCellHalf }); // 当たり判定は今まで通りセル全体
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

		// STEP: 枠(Door_Frame.obj)の高さは天井まで届かない(実測kFrameTopY)ので、
		// 枠の上から天井までを塞ぐ「まぐさ(鴨居)」ブロックを追加。これが
		// ドア上部に隙間が見える不具合の原因だった。厚み軸は袖壁と同じ
		// thicknessOffset/thicknessHalfを使って、壁面とツライチに揃える。
		const float kFrameTopY = 2.175f; // Door_Frame.objの実測高さ(分割スクリプト参照)
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

	// 廃墟に転がってるだけのクレート。テクスチャが手元に無かったので単色
	// のプレースホルダー -- crate.h参照。
	void SpawnCrate(const Vector3& position, float yRotation)
	{
		Crate* crate = Manager::AddGameObject<Crate>();
		crate->SetPosition(position);
		crate->SetRotation({ 0.0f, yRotation, 0.0f });
	}

	// 壁に貼り付けるツタの飾り(boston+ivy.obj)。wallYRotationは壁の向き
	// (光源と同じ0/XM_PIDIV2の判定基準)。テクスチャが手元に無かったので
	// こちらも単色のプレースホルダー -- ivy.h参照。
	void SpawnIvy(const Vector3& position, float wallYRotation)
	{
		Ivy* ivy = Manager::AddGameObject<Ivy>();
		ivy->SetPosition(position);
		ivy->SetRotation({ 0.0f, wallYRotation, 0.0f });
	}
}

void Map::Init()
{
	Door* gimmickDoor = nullptr;

	// 壁ブロックをまとめて生成(引き延ばし) -- 元は下のswitch文中で1セル
	// ずつBoxを出していたが、後々ドアモデルなどを壁に埋め込みやすいように
	// 隙間なく繋がった少数の大きなBoxへ変更した(当たり判定・見た目とも
	// 元の形と完全に同じであることをPythonで検証済み -- SpawnMergedWalls参照)。
	SpawnMergedWalls();

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
				// 個別のBoxではなく、Map::Init()冒頭のSpawnMergedWalls()で
				// まとめて生成済み -- ここでは何もしない。
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
				Door* door = SpawnDoorway(row, col, true);
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
				Door* door = SpawnDoorway(row, col, false);
				door->SetRequiredKey(4); // final key, from the 'F' box above
				door->SetIsExit(true);   // finishing its open animation triggers CLEAR -- see door.cpp
				break;
			}
			default:
				break; // plain floor; Field is the floor mesh
			}
		}
	}

	// 天井も壁と同じ理由(隙間を無くす・後々扱いやすくする)でまとめる。
	SpawnMergedCeiling();

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

	// 廃墟の散らかり物 -- 部屋ごとにスツール2個+壁に立てかけたパレット1個。
	// 綺麗に並べず、部屋の中でバラバラの位置・向きになるようにしてある。

	// room A (colBase=1, rowBase=1) -- パレットは西側の壁に立てかけ。5個: 立ち
	// スツール2+倒れたスツール1+立てかけパレット1+転がったパレット1。
	// K(鍵, col2 row2)・X(スイッチ, col1 row3)とは被らない位置を選んである。
	SpawnStool(CellCenter(1, 1) + Vector3(0.6f, 0.0f, -0.5f), 0.9f);
	SpawnStool(CellCenter(3, 3) + Vector3(-0.4f, 0.0f, 0.7f), 2.5f);
	SpawnFallenStool(CellCenter(3, 1) + Vector3(-0.5f, 0.0f, 0.4f), 1.0f);
	SpawnLeaningPallet(CellCenter(1, 2) + Vector3(-4.73f, 0.0f, 0.2f), XM_PIDIV2);
	SpawnFlatPallet(CellCenter(2, 3) + Vector3(0.3f, 0.0f, -0.6f), 0.6f);

	// room B (colBase=1, rowBase=5) -- パレットは南側の壁に立てかけ。5個。
	// M(アイテム, col2 row5)・G(ギミックドア, col4 row6)・F(アイテム箱, col1
	// row7)とは被らない位置を選んである。
	SpawnStool(CellCenter(3, 5) + Vector3(0.5f, 0.0f, -0.6f), 1.2f);
	SpawnStool(CellCenter(2, 7) + Vector3(-0.3f, 0.0f, 0.5f), 3.0f);
	SpawnFallenStool(CellCenter(1, 6) + Vector3(0.4f, 0.0f, -0.3f), 2.2f);
	SpawnLeaningPallet(CellCenter(2, 7) + Vector3(0.3f, 0.0f, 4.57f), 0.0f);
	SpawnFlatPallet(CellCenter(3, 6) + Vector3(-0.4f, 0.0f, 0.5f), 1.8f);

	// room C (colBase=8, rowBase=1) -- パレットは東側の壁に立てかけ。5個。
	// R(アイテム, col9 row1)とは被らない位置を選んである。
	SpawnStool(CellCenter(10, 1) + Vector3(-0.5f, 0.0f, 0.6f), 1.7f);
	SpawnStool(CellCenter(8, 3) + Vector3(0.6f, 0.0f, -0.4f), 0.3f);
	SpawnFallenStool(CellCenter(9, 3) + Vector3(0.3f, 0.0f, 0.5f), 0.4f);
	SpawnLeaningPallet(CellCenter(10, 2) + Vector3(4.57f, 0.0f, -0.2f), XM_PIDIV2);
	SpawnFlatPallet(CellCenter(8, 1) + Vector3(-0.4f, 0.0f, -0.3f), 2.6f);

	// room N (colBase=8, rowBase=5) -- パレットは南側の壁に立てかけ。5個。
	// P(アイテム, col9 row5)とは被らない位置を選んである。
	SpawnStool(CellCenter(10, 6) + Vector3(0.4f, 0.0f, -0.6f), 2.0f);
	SpawnStool(CellCenter(8, 7) + Vector3(-0.5f, 0.0f, 0.5f), 0.8f);
	SpawnFallenStool(CellCenter(10, 7) + Vector3(-0.3f, 0.0f, -0.5f), 3.0f);
	SpawnLeaningPallet(CellCenter(9, 7) + Vector3(0.2f, 0.0f, 4.57f), 0.0f);
	SpawnFlatPallet(CellCenter(8, 6) + Vector3(0.4f, 0.0f, 0.3f), 1.2f);

	// おまけ: 部屋の中だけでなく、中央の廊下にも倒れたスツールを少し
	// 転がしておく(廃墟感を部屋の外にも広げるための追加分)。
	SpawnStool(CellCenter(5, 2) + Vector3(0.3f, 0.0f, -0.4f), 1.4f);
	SpawnStool(CellCenter(6, 6) + Vector3(-0.3f, 0.0f, 0.4f), 3.6f);

	// 新しく追加したプロップ -- クレート2個(ガラクタを少し増やす)とツタの
	// 壁飾り1個(部屋Nの北壁)。テクスチャが手元に無かった2つ(Crate_2x/
	// boston+ivy)は単色のプレースホルダー -- crate.h/ivy.hのコメント参照。
	SpawnCrate(CellCenter(1, 5) + Vector3(-0.3f, 0.0f, 0.4f), 0.7f);   // room B
	SpawnCrate(CellCenter(9, 2) + Vector3(-0.2f, 0.0f, -0.3f), 2.1f);  // room C
	SpawnIvy(CellCenter(9, 5) + Vector3(0.0f, 0.0f, -(CELL_SIZE / 2.0f - 0.15f)), 0.0f); // room N 北壁

	// STEP: 「廃墟っぽさを増やしてほしい」との要望で追加 -- 北壁に面した
	// 部屋(北側が#で塞がれてる部屋)に同じ貼り方でツタを増やして、あちこち
	// 侵食されてる感じを出す。既存の1本(room N)と同じオフセット/回転の
	// 組み合わせなので、向きが逆になる心配はない。
	SpawnIvy(CellCenter(2, 1) + Vector3(0.0f, 0.0f, -(CELL_SIZE / 2.0f - 0.15f)), 0.0f); // room A 北壁
	SpawnIvy(CellCenter(9, 1) + Vector3(0.0f, 0.0f, -(CELL_SIZE / 2.0f - 0.15f)), 0.0f); // room C 北壁
	SpawnIvy(CellCenter(2, 5) + Vector3(0.0f, 0.0f, -(CELL_SIZE / 2.0f - 0.15f)), 0.0f); // room B 北壁
	SpawnIvy(CellCenter(8, 5) + Vector3(0.3f, 0.0f, -(CELL_SIZE / 2.0f - 0.15f)), 0.0f); // room N 北壁(もう1本、密度アップ)
}
