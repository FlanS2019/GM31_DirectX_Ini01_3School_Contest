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
void Title::Init()
{
	//タイトル画面の背景画像を表示するためのPolygon2Dオブジェクトを作成
	Manager::AddGameObject<Polygon2D>()->Init(0.0f, 0.0f, SCREEN_WIDTH,SCREEN_HEIGHT, L"texture\\image.png");

	// STEP24: 追加仕様書14項「タイトル画面に設定ボタンを追加」。
	// 以前はEnterキー1つで即ゲーム開始するだけだったが、選択式の
	// メニュー(TitleMenu)に置き換えた -- Enter処理はTitleMenu::Update()
	// 側に移したので、下のTitle::Update()は空になっている。
	// SettingsScreenはGame::Init()側とは別インスタンス(settingsScreen.h
	// のクラスコメント参照)。
	Manager::AddGameObject<TitleMenu>();
	Manager::AddGameObject<SettingsScreen>();
}

void Title::Uninit()
{
}

void Title::Update()
{
	// STEP24: 「ゲームスタート」「設定」の選択・決定はTitleMenu::Update()
	// が処理する(このシーンにTitleMenuを追加済み -- Init()参照)。
}

void Title::Draw()
{
}
