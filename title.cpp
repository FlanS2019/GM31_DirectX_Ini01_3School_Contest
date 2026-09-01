//title.cpp
#include "main.h"
#include "manager.h"
#include "renderer.h"
#include "Input.h"
#include "title.h"
#include "polygon2d.h"
#include "Game.h"
void Title::Init()
{
	//タイトル画面の背景画像を表示するためのPolygon2Dオブジェクトを作成
	Manager::AddGameObject<Polygon2D>()->Init(0.0f, 0.0f, SCREEN_WIDTH,SCREEN_HEIGHT, L"texture\\image.png");
}

void Title::Uninit()
{
}

void Title::Update()
{
	if(Input::GetKeyTrigger(VK_RETURN))
	{
		Manager::ChangeScene<Game>();
	}
}

void Title::Draw()
{
}
