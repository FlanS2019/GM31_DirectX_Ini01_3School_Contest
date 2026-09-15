#include "main.h"
#include "manager.h"
#include "howToPlay.h"
#include "menuBackgroundScene.h"
#include "polygon2d.h"
#include "howToPlayMenu.h"
#include "menuSound.h" // STEP27

void HowToPlay::Init()
{
	// タイトル/リザルトと同じ背景を流用(既存仕様書9項の「OP・説明・
	// リザルトの背景を共通化」方針をそのまま踏襲)。
	// STEP29: 2DのPolygon2Dをやめて、タイトルと同じ3D背景に統一。
	MenuBackgroundScene::Init();

	// STEP26: 操作アイコン(WASD/マウス/E/ESC)。Polygon2D::Init()はどれも
	// m_Layer=9固定になる(polygon2d.cpp参照)が、背景の直後に追加して
	// いるので同じレイヤー内では背景より後に描かれ、正しく背景の上に
	// 乗る(Manager::Draw()は同一レイヤー内では追加した順に描画する)。
	Manager::AddGameObject<Polygon2D>()->Init(300.0f, 240.0f, 240.0f, 180.0f, L"texture\\icon_wasd.png"); // STEP34: 1.5x
	Manager::AddGameObject<Polygon2D>()->Init(1050.0f, 225.0f, 150.0f, 210.0f, L"texture\\icon_mouse.png"); // STEP34: 1.5x
	Manager::AddGameObject<Polygon2D>()->Init(352.5f, 645.0f, 135.0f, 135.0f, L"texture\\icon_e.png"); // STEP34: 1.5x
	Manager::AddGameObject<Polygon2D>()->Init(1035.0f, 645.0f, 195.0f, 135.0f, L"texture\\icon_esc.png"); // STEP34: 1.5x

	// 見出し・アイコン下のラベル・「戻る」の選択処理はHowToPlayMenu側で
	// まとめて描く(TitleMenu/ResultMenuと同じ、m_Layer=10のGameObject
	// パターン -- howToPlayMenu.h参照)。
	Manager::AddGameObject<HowToPlayMenu>();
	Manager::AddGameObject<MenuSound>(); // STEP27: メニュー操作音
}

void HowToPlay::Uninit()
{
	MenuBackgroundScene::Uninit();
}

void HowToPlay::Update()
{
	// STEP26: 「戻る」の決定処理はHowToPlayMenu::Update()に移した
	// (title.cpp/result.cppと同じパターン)。
}

void HowToPlay::Draw()
{
}
