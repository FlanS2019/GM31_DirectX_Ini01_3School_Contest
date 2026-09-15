//title.cpp
#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "Input.h"
#include "title.h"
#include "polygon2d.h"
#include "Game.h"
#include "titleMenu.h"
#include "settingsScreen.h"
#include "menuSound.h" 
#include "titleBgm.h" 
#include "titleLogo.h" 
void Title::Init()
{
	MenuBackgroundScene::Init();

	Manager::AddGameObject<TitleLogo>();

	Manager::AddGameObject<TitleMenu>();
	Manager::AddGameObject<SettingsScreen>();
	Manager::AddGameObject<MenuSound>(); 
	Manager::AddGameObject<TitleBgm>(); 
}

void Title::Uninit()
{
	MenuBackgroundScene::Uninit();
}

void Title::Update()
{
}

void Title::Draw()
{
}
