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

	// Flat-color rounded-rect panel -- used as the persistent inventory
	// frame at the bottom of the screen (STEP12). Independent of DrawText's
	// per-call panel background, so it can be drawn once behind several
	// DrawText calls instead of getting its own box per line.
	static void DrawPanel(float x, float y, float width, float height);

	// STEP15: permanent atmosphere -- a radial-gradient darkening toward
	// the screen edges (brush built once in Init()); strength (0..1) is
	// just its opacity, so callers (horror.h) can vary it per frame
	// cheaply. Draws nothing if strength <= 0 or the brush failed to
	// build.
	static void DrawVignette(float strength);

	// STEP15: flat full-screen color flash -- jump scares / a scripted
	// "power goes out" beat. horror.h owns the fade timer and passes the
	// alpha down each frame; draws nothing if a <= 0.
	static void DrawFullScreenTint(float r, float g, float b, float a);

	// STEP16: small flat-shape primitives -- there's no image/texture
	// loading pipeline in this engine for a real item thumbnail, so
	// interact.cpp's hotbar draws each item as a little vector icon built
	// out of these instead of a text label (see DrawItemIcon() there).
	static void DrawFilledRect(float x, float y, float w, float h, float r, float g, float b, float a = 1.0f);
	static void DrawFilledEllipse(float cx, float cy, float radiusX, float radiusY, float r, float g, float b, float a = 1.0f);
	static void DrawRingEllipse(float cx, float cy, float radiusX, float radiusY, float strokeWidth, float r, float g, float b, float a = 1.0f);
	static void DrawFilledPolygon(const float* xs, const float* ys, int count, float r, float g, float b, float a = 1.0f);
};