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
#include "resultMenu.h" // STEP25
#include "menuSound.h" // STEP27
void result::Init()
{
	//結果画面の背景画像を表示するためのPolygon2Dオブジェクトを作成
	//村松、単語間違えるなよ。背景くれたのはありがとう。
	// STEP29: ↑の2D背景をやめて、タイトルと同じ3D背景(MenuBackgroundScene)
	// に統一する。
	MenuBackgroundScene::Init();

	// STEP25: 「脱出成功」見出し+生還時間+見つけた鍵の数+「タイトルへ戻る」
	// をまとめて表示する選択式メニュー(TitleMenuと同じパターン)。
	Manager::AddGameObject<ResultMenu>();
	Manager::AddGameObject<MenuSound>(); // STEP27: メニュー操作音
}

void result::Uninit()
{
	MenuBackgroundScene::Uninit();
}

void result::Update()
{
	// STEP25: 「タイトルへ戻る」の決定処理はResultMenu::Update()に移した
	// (TitleMenuに合わせたパターン -- title.cppのUpdate()も参照)。
}

void result::Draw()
{
}
