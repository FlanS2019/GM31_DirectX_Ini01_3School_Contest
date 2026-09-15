#include "main.h"
#include "manager.h"
#include "howToPlay.h"
#include "menuBackgroundScene.h"
#include "polygon2d.h"
#include "howToPlayMenu.h"
#include "menuSound.h"

void HowToPlay::Init()
{
	MenuBackgroundScene::Init();

	Manager::AddGameObject<Polygon2D>()->Init(300.0f, 240.0f, 240.0f, 180.0f, L"texture\\icon_wasd.png"); 
	Manager::AddGameObject<Polygon2D>()->Init(1050.0f, 225.0f, 150.0f, 210.0f, L"texture\\icon_mouse.png"); 
	Manager::AddGameObject<Polygon2D>()->Init(352.5f, 645.0f, 135.0f, 135.0f, L"texture\\icon_e.png"); 
	Manager::AddGameObject<Polygon2D>()->Init(1035.0f, 645.0f, 195.0f, 135.0f, L"texture\\icon_esc.png"); 

	Manager::AddGameObject<HowToPlayMenu>();
	Manager::AddGameObject<MenuSound>(); //ÉÅÉjÉÖÅ[ëÄçÏâπ
}

void HowToPlay::Uninit()
{
	MenuBackgroundScene::Uninit();
}

void HowToPlay::Update()
{
}

void HowToPlay::Draw()
{
}
