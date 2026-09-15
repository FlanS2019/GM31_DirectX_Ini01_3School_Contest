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

	static void DrawTextAlpha(const char* text, float x, float y,
		float size, bool centered, float alpha);

	static void DrawPanel(float x, float y, float width, float height);

	static void DrawVignette(float strength);

	static void DrawFullScreenTint(float r, float g, float b, float a);

	static void DrawFilledRect(float x, float y, float w, float h, float r, float g, float b, float a = 1.0f);
	static void DrawFilledEllipse(float cx, float cy, float radiusX, float radiusY, float r, float g, float b, float a = 1.0f);
	static void DrawRingEllipse(float cx, float cy, float radiusX, float radiusY, float strokeWidth, float r, float g, float b, float a = 1.0f);
	static void DrawFilledPolygon(const float* xs, const float* ys, int count, float r, float g, float b, float a = 1.0f);
};