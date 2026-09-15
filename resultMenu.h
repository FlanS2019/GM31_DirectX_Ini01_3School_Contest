#pragma once

#include "gameObject.h"

// STEP25: result.cppの拡張。TitleMenuと全く同じ理由・同じパターン
// (選択式メニュー+m_Layerの明示指定)を踏襲している -- 詳しくは
// titleMenu.hのコメント参照。
//
// 表示する時間/鍵の数はGameStats(静的クラス)から読む -- GameStats.hの
// クラスコメント参照。
class ResultMenu : public GameObject
{
private:
	int m_Selected = 0;

public:
	// STEP25: result.cppの背景(Polygon2D)もタイトルと同じくm_Layer=9で
	// 描画される(polygon2d.cpp参照)。TitleMenuで踏んだのと同じ地雷を
	// 避けるため、最初からこれより大きいレイヤーにしておく。
	void Init() override { m_Selected = 0; m_Layer = 10; }
	void Uninit() override {}
	void Update() override;
	void Draw() override;
};
