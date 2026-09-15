#include "main.h"
#include "menuBackgroundScene.h"
#include "manager.h"
#include "menuCamera.h"
#include "light.h"
#include "field.h"
#include "Map.h"
#include "polygon2d.h" // STEP30: 灰色フィルター用

void MenuBackgroundScene::Init()
{
	// STEP29: カメラ→ライト→床→マップの順で用意する。Manager::Draw()は
	// まずCamera::Draw()でView/Projection行列を積んでから他のレイヤーを
	// 描画するので、この並び自体に依存関係はないが分かりやすさのため
	// カメラを先頭にしている。
	Manager::AddGameObject<MenuCamera>();
	Manager::AddGameObject<Light>();
	Manager::AddGameObject<Field>();
	Manager::AddGameObject<Map>();

	// STEP30: 3D背景全体に薄い灰色フィルターを重ねる。Polygon2D::Init()は
	// どれもm_Layer=9固定(polygon2d.cpp参照)なので、Map追加の直後(=この
	// レイヤー内では一番手前)に置けば3D背景全体を覆い、かつ各シーンが
	// あとから追加するアイコン/ボタン等(m_Layer=10)の下に収まる。
	Manager::AddGameObject<Polygon2D>()->Init(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, L"texture\\filter_gray.png");
}

void MenuBackgroundScene::Uninit()
{
}
