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
#include "menuSound.h" // STEP27
#include "titleBgm.h" // STEP28
void Title::Init()
{
	// STEP29: 背景を2DのPolygon2Dから3D背景(カメラ+ライト+床+マップ)に
	// 変更。操作説明/リザルトと共通化するため実体はMenuBackgroundScene
	// 側にまとめてあり、ここでは呼ぶだけ(menuBackgroundScene.cpp参照)。
	MenuBackgroundScene::Init();

	// STEP24: 追加仕様書14項「タイトル画面に設定ボタンを追加」。
	// 以前はEnterキー1つで即ゲーム開始するだけだったが、選択式の
	// メニュー(TitleMenu)に置き換えた -- Enter処理はTitleMenu::Update()
	// 側に移したので、下のTitle::Update()は空になっている。
	// SettingsScreenはGame::Init()側とは別インスタンス(settingsScreen.h
	// のクラスコメント参照)。
	Manager::AddGameObject<TitleMenu>();
	Manager::AddGameObject<SettingsScreen>();
	Manager::AddGameObject<MenuSound>(); // STEP27: メニュー操作音
	Manager::AddGameObject<TitleBgm>(); // STEP28: タイトル画面BGM
}

void Title::Uninit()
{
	MenuBackgroundScene::Uninit();
}

void Title::Update()
{
	// STEP24: 「ゲームスタート」「設定」の選択・決定はTitleMenu::Update()
	// が処理する(このシーンにTitleMenuを追加済み -- Init()参照)。
}

void Title::Draw()
{
}
