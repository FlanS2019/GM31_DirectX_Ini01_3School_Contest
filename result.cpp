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
void result::Init()
{
	//村松、単語間違えるなよ。背景くれたのはありがとう。
	MenuBackgroundScene::Init();

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
