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
};