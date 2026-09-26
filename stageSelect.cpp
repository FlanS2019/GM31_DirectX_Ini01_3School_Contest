//stageSelect.cpp
#include "main.h"
#include "manager.h"
#include "stageSelect.h"
#include "stageSelectMenu.h"
#include "settingsScreen.h"
#include "menuSound.h"
#include "titleBgm.h"

void StageSelect::Init()
{
	MenuBackgroundScene::Init();

	Manager::AddGameObject<StageSelectMenu>();
	Manager::AddGameObject<SettingsScreen>();
	Manager::AddGameObject<MenuSound>();
	Manager::AddGameObject<TitleBgm>();
}

void StageSelect::Uninit()
{
	MenuBackgroundScene::Uninit();
}
