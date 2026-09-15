#pragma once

#include "gameObject.h"

// STEP24: 追加仕様書14項「タイトル画面: 既存の3ボタンに『設定』ボタンを
// 追加し、4ボタン構成にする」に対応する第一段階。
//
// 実装時点でのタイトル画面(title.cpp)は背景画像1枚+Enterキーで即
// ゲーム開始、というごく単純なものしかなく、既存仕様書側の「操作説明」
// (HowToPlayScene)やタイトルからの「ゲーム終了」ボタンもまだ実装されて
// いない。そのため今回はこの追加仕様書自身が要求する範囲 --
// 「設定画面をタイトルからも開けるようにする」-- に絞り、選択式の
// メニューを「ゲームスタート」「設定」の2項目で新設した。操作説明/
// ゲーム終了ボタンを含む本来の4(または5)ボタン構成は、HowToPlayScene
// 等が実装された時点でこのクラスに項目を追加するだけで拡張できる。
//
// Title::Init()から一度だけ追加される。SettingsScreenも同様にTitle::
// Init()が自分用のインスタンスを追加する(settingsScreen.hのクラス
// コメント参照) -- こちらのTitleMenuはそれをManager::GetGameObject<
// SettingsScreen>()経由で参照するだけ。
//
// Title::Draw()はScene::Draw()自体がManagerから一切呼ばれない(死んで
// いるオーバーライド)ため使えない -- 代わりにこのGameObjectとしての
// 普通のDraw()の中で直接Hud::Begin()/End()を呼ぶ。Titleシーンには他に
// Hud::Begin()/End()を使うオブジェクトが無いので、Interact/Horrorの
// ようにDrawUI()を外から呼んでもらう間接パターンは不要。
class TitleMenu : public GameObject
{
private:
	int m_Selected = 0;

public:
	void Init() override { m_Selected = 0; }
	void Uninit() override {}
	void Update() override;
	void Draw() override;
};
