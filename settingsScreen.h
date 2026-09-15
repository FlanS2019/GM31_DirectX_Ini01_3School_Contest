#pragma once

#include "gameObject.h"

// STEP24: 追加仕様書13/14項「設定画面」。タイトル画面(TitleMenu経由)と
// ゲームプレイ中(PauseMenu経由)の両方から使う共通実装。
//
// シーンが変わるとManager::Update()がg_GameObjectを全部破棄してしまう
// (Scene切り替え=全オブジェクト作り直し)ため、SettingsScreenという
// "1つの"インスタンスをTITLEとGAMEの間で使い回すことはできない。
// その代わりTitle::Init()とGame::Init()がそれぞれ自分のSettingsScreen
// インスタンスを1つずつ追加する(PauseMenu/TitleMenu/Horror/Interactと
// 同じAddGameObject<T>()パターン)。中身(GetBgmVolume()等が読みに行く
// GameSettingsの値)は両方のインスタンスで共通なので、「タイトルから開いた
// 設定」と「ポーズから開いた設定」は見た目も挙動も完全に同じになる
// (追加仕様書14項の「戻り先以外の挙動を変えない」方針)。「閉じたら
// 開いた場所へ戻る」のも、閉じた瞬間そのインスタンスのm_Open=falseに
// なるだけで、呼び出し元(TitleMenu/PauseMenu)側が自然に制御を取り戻す
// ので、戻り先IDを別途持ち回る必要がない。
class SettingsScreen : public GameObject
{
private:
	bool m_Open = false;
	int m_Selected = 0;

public:
	void Init() override {}
	void Uninit() override {}
	void Update() override;
	void Draw() override {} // 実際の描画はDrawUI() -- pauseMenu.hと同じパターン

	// ポーズ中(Game::Init()側のインスタンス)でも入力を受け付け続ける
	// 必要があるため。TitleシーンではManager::IsPaused()は常にfalseなので
	// このオーバーライドは無関係(=無害)。
	bool UpdatesWhilePaused() const override { return true; }

	void Open() { m_Open = true; m_Selected = 0; }
	void Close() { m_Open = false; }
	bool IsOpen() const { return m_Open; }

	// 呼び出し元のHud::Begin()/End()の中から呼んでもらう用。
	void DrawUI();
};
