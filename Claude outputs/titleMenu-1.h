#pragma once

#include "gameObject.h"

// STEP26: 追加仕様書14項「タイトル画面: 既存の3ボタンに『設定』ボタンを
// 追加し、4ボタン構成にする」に完全対応する版。STEP24時点では操作説明
// 画面(HowToPlayScene)がまだ存在せず、タイトルの「ゲーム終了」ボタンも
// 無かったため、「ゲームスタート」「設定」の2項目に絞っていた(この
// ファイルの旧バージョンのコメント参照)。howToPlay.h/cppを新規に用意
// したので、本来の4ボタン構成(ゲームスタート/操作説明/設定/ゲーム終了)
// に拡張する。
//
// 「ゲーム終了」はPauseMenuの確認画面パターン(pauseMenu.h参照)をそのまま
// 踏襲 -- 誤操作防止のためYes/No確認を挟む。
class TitleMenu : public GameObject
{
public:
	enum class State
	{
		Root,
		ConfirmQuit,
	};

private:
	State m_State = State::Root;
	int m_Selected = 0;

public:
	// STEP24: タイトルの背景(Polygon2D)はm_Layer=9で描画される
	// (polygon2d.cpp参照)。Manager::Draw()はレイヤー番号の小さい順に
	// 描画するため、TitleMenuが既定のm_Layer=1のままだと、背景画像より
	// "先に"メニュー文字を描いてしまい、その直後に描かれる不透明な
	// 背景画像に上書きされて画面から消えてしまう(ロジックは正常に動くが
	// 見た目だけ何も出ない、という不具合の原因だった)。背景より確実に
	// 後で描かれるよう、ここでm_Layerを背景より大きい値にしておく。
	void Init() override { m_State = State::Root; m_Selected = 0; m_Layer = 10; }
	void Uninit() override {}
	void Update() override;
	void Draw() override;
};
