#pragma once

#include "menuBackgroundScene.h"

// STEP26: 追加仕様書14項「タイトル画面: 既存3ボタン+設定で4ボタン構成」に
// 対応するために新規に用意した「操作説明」画面。元のプロジェクトには
// この画面自体が存在せず(タイトルはEnterキーで即スタートするだけ
// だった)、STEP24時点のTitleMenuは「ゲームスタート」「設定」の2項目に
// 絞っていた(titleMenu.hの旧コメント参照)。今回このScene派生クラスを
// 追加したことで、TitleMenuを本来の4ボタン構成に拡張できる。
// STEP29: 背景をタイトルと共通の3D背景にするため、Sceneではなく
// MenuBackgroundSceneを継承するように変更。
class HowToPlay : public MenuBackgroundScene
{
public:
	void Init() override;
	void Uninit() override;
	void Update() override;
	void Draw() override;
};
