#pragma once

#include "gameObject.h"

// STEP26: howToPlay.cppの拡張。TitleMenu/ResultMenuと全く同じ理由・同じ
// パターン(選択式メニュー+m_Layerの明示指定)を踏襲している -- 詳しくは
// titleMenu.hのコメント参照。
class HowToPlayMenu : public GameObject
{
private:
	int m_Selected = 0;

public:
	// howToPlay.cppの背景・操作アイコン(いずれもPolygon2D)はm_Layer=9で
	// 描画される。TitleMenuで踏んだのと同じ地雷を避けるため、最初から
	// これより大きいレイヤーにしておく。
	void Init() override { m_Selected = 0; m_Layer = 10; }
	void Uninit() override {}
	void Update() override;
	void Draw() override;
};
