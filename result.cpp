//result.cpp
#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "Input.h"
#include "result.h"
#include "menuBackgroundScene.h"
#include "polygon2d.h"
#include "title.h"
#include "Game.h"
#include "resultMenu.h"
#include "menuSound.h"
#include "gameSettings.h" // STEP(stage-select): SetStageCleared() below
void result::Init()
{
	//ë∫èºÅAíPåÍä‘à·Ç¶ÇÈÇ»ÇÊÅBîwåiÇ≠ÇÍÇΩÇÃÇÕÇ†ÇËÇ™Ç∆Ç§ÅB
	MenuBackgroundScene::Init();

	// STEP(stage-select): reaching this screen means the player opened the
	// 'E' finale door -- stage 0 (îpö–) counts as cleared from here on, which
	// is what unlocks stage 1 (ínâ∫) on the new Stage Select screen.
	GameSettings::SetStageCleared(0);

	Manager::AddGameObject<ResultMenu>();
	Manager::AddGameObject<MenuSound>(); 
}

void result::Uninit()
{
	MenuBackgroundScene::Uninit();
}

void result::Update()
{
}

void result::Draw()
{
}
