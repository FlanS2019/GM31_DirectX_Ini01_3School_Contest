#pragma once
#include "gameObject.h"

class Polygon2D;

// STEP40: "RUINS STUDIO"ロゴ(logo_splash.png)をフェードイン→静止→
// フェードアウトで見せてからタイトルへ進む起動スプラッシュ。GameStartText
// (gameStartText.h)と同じ3段階タイマーのパターン。何かキーを押すか
// マウスをクリックするとその場でフェードアウト側にスキップできる
// (即座にカットすると唐突なので、フェードアウト自体は省略しない)。
class SplashLogo : public GameObject
{
private:
	Polygon2D* m_Logo = nullptr;
	float m_Timer = 0.0f;
	bool m_Skipped = false;
	bool m_ChangedScene = false;
public:
	void Init() override;
	void Update() override;
};
