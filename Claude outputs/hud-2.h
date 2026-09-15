#pragma once

class Hud
{
public:
	static void Init();
	static void Uninit();

	static void Begin(); // call once, right before the first DrawText() this frame
	static void End();   // call once, right after the last DrawText() this frame

	static void DrawText(const char* text, float x, float y,
		float size = 22.0f, bool centered = false);

	// STEP32: 「ゲームスタート」演出用にフェードイン/アウトさせたい
	// テキスト向けの版。DrawText()と違って背景パネルは付けない
	// (フェードする文字だけをふわっと出したいので)。alphaは0..1。
	static void DrawTextAlpha(const char* text, float x, float y,
		float size, bool centered, float alpha);

	// Flat-color rounded-rect panel -- used as the persistent inventory
	// frame at the bottom of the screen (STEP12). Independent of DrawText's
	// per-call panel background, so it can be drawn once behind several
	// DrawText calls instead of getting its own box per line.
	static void DrawPanel(float x, float y, float width, float height);

	// STEP34: PauseMenu/SettingsScreenの全画面ディム(暗転)用。DrawPanel()
	// は常に固定の黒(不透明度0.55)の角丸パネルだが、こちらは色/不透明度を
	// 呼び出し側で指定できる、角丸なしの単色矩形。
	static void DrawFilledRect(float x, float y, float width, float height,
		float r, float g, float b, float a);
};