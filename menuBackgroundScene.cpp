#include "main.h"
#include "menuBackgroundScene.h"
#include "manager.h"
#include "menuCamera.h"
#include "light.h"
#include "field.h"
#include "Map.h"
#include "polygon2d.h" 

void MenuBackgroundScene::Init()
{
	Manager::AddGameObject<MenuCamera>();
	Manager::AddGameObject<Light>();
	Manager::AddGameObject<Field>();
	Manager::AddGameObject<Map>();

	Manager::AddGameObject<Polygon2D>()->Init(0.0f, 0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, L"texture\\filter_gray.png");
}

void MenuBackgroundScene::Uninit()
{
}
