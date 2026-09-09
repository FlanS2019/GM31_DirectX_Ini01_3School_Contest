#include "main.h"
#include "hud.h"
#include "renderer.h"

#include <d2d1.h>
#include <dwrite.h>
#include <vector>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

namespace
{
	ID2D1Factory* g_D2DFactory = nullptr;
	ID2D1RenderTarget* g_D2DRenderTarget = nullptr;
	IDWriteFactory* g_DWriteFactory = nullptr;
	IDWriteTextFormat* g_TextFormat = nullptr;
	ID2D1SolidColorBrush* g_TextBrush = nullptr;
	ID2D1SolidColorBrush* g_ShadowBrush = nullptr;
	ID2D1SolidColorBrush* g_PanelBrush = nullptr;

	bool g_Ready = false; // stays false (DrawText becomes a silent no-op) if any of the setup below fails
}

namespace
{
	void LogHudInitFailure(const char* step, HRESULT hr)
	{
		char buf[160];
		sprintf_s(buf, "[Hud] Init FAILED at %s (hr=0x%08X) -- on-screen prompt will never draw.\n", step, (unsigned int)hr);
		OutputDebugStringA(buf);
	}
}

void Hud::Init()
{
	HRESULT hr;

	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_D2DFactory);
	if (FAILED(hr)) { LogHudInitFailure("D2D1CreateFactory", hr); return; }

	IDXGISurface* backBufferSurface = nullptr;
	hr = Renderer::GetSwapChain()->GetBuffer(0, __uuidof(IDXGISurface), (void**)&backBufferSurface);
	if (FAILED(hr)) { LogHudInitFailure("GetBuffer(IDXGISurface)", hr); return; }

	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED));

	hr = g_D2DFactory->CreateDxgiSurfaceRenderTarget(backBufferSurface, &props, &g_D2DRenderTarget);
	backBufferSurface->Release();
	if (FAILED(hr)) { LogHudInitFailure("CreateDxgiSurfaceRenderTarget", hr); return; }

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&g_DWriteFactory);
	if (FAILED(hr)) { LogHudInitFailure("DWriteCreateFactory", hr); return; }

	hr = g_DWriteFactory->CreateTextFormat(
		L"MS Gothic", nullptr,
		DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL,
		22.0f, L"ja-jp", &g_TextFormat);
	if (FAILED(hr)) { LogHudInitFailure("CreateTextFormat", hr); return; }

	g_TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	g_TextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

	g_D2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &g_TextBrush);
	g_D2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &g_ShadowBrush);
	g_D2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black, 0.55f), &g_PanelBrush);

	g_Ready = true;
	OutputDebugStringA("[Hud] Init OK\n");
}

void Hud::Uninit()
{
	g_Ready = false;

	if (g_PanelBrush) { g_PanelBrush->Release();      g_PanelBrush = nullptr; }
	if (g_ShadowBrush) { g_ShadowBrush->Release();     g_ShadowBrush = nullptr; }
	if (g_TextBrush) { g_TextBrush->Release();       g_TextBrush = nullptr; }
	if (g_TextFormat) { g_TextFormat->Release();      g_TextFormat = nullptr; }
	if (g_DWriteFactory) { g_DWriteFactory->Release();   g_DWriteFactory = nullptr; }
	if (g_D2DRenderTarget) { g_D2DRenderTarget->Release(); g_D2DRenderTarget = nullptr; }
	if (g_D2DFactory) { g_D2DFactory->Release();      g_D2DFactory = nullptr; }
}

void Hud::Begin()
{
	if (!g_Ready) return;
	g_D2DRenderTarget->BeginDraw();
}

void Hud::End()
{
	if (!g_Ready) return;

	HRESULT hr = g_D2DRenderTarget->EndDraw();

	if (FAILED(hr))
	{
		char buf[128];
		sprintf_s(buf, "[Hud] EndDraw failed (0x%08X) -- HUD text may stop updating.\n", (unsigned int)hr);
		OutputDebugStringA(buf);
	}
}

void Hud::DrawText(const char* text, float x, float y, float size, bool centered)
{
	if (!g_Ready)
	{
		static bool loggedOnce = false;
		if (!loggedOnce)
		{
			OutputDebugStringA("[Hud] DrawText called but Hud is not ready (Init() failed earlier) -- nothing will draw.\n");
			loggedOnce = true;
		}
		return;
	}

	if (!g_Ready || !text || !text[0]) return;

	int wlen = MultiByteToWideChar(CP_ACP, 0, text, -1, nullptr, 0);
	if (wlen <= 0) return;

	std::vector<wchar_t> wide((size_t)wlen);
	MultiByteToWideChar(CP_ACP, 0, text, -1, wide.data(), wlen);
	UINT32 length = (UINT32)(wlen - 1); // DWrite's length excludes the null terminator

	IDWriteTextLayout* layout = nullptr;
	HRESULT hr = g_DWriteFactory->CreateTextLayout(wide.data(), length, g_TextFormat,
		1000.0f, size * 1.6f, &layout);
	if (FAILED(hr) || !layout) return;

	DWRITE_TEXT_RANGE fullRange = { 0, length };
	layout->SetFontSize(size, fullRange);

	DWRITE_TEXT_METRICS metrics{};
	layout->GetMetrics(&metrics);

	float left = centered ? (x - metrics.width * 0.5f) : x;

	const float padX = 10.0f, padY = 6.0f;
	D2D1_ROUNDED_RECT panel = D2D1::RoundedRect(
		D2D1::RectF(left - padX, y - padY, left + metrics.width + padX, y + metrics.height + padY),
		4.0f, 4.0f);
	g_D2DRenderTarget->FillRoundedRectangle(panel, g_PanelBrush);

	// 1px drop shadow, then the real text on top -- cheap stand-in for a
	// proper outline/stroke pass.
	g_D2DRenderTarget->DrawTextLayout(D2D1::Point2F(left + 1.0f, y + 1.0f), layout, g_ShadowBrush);
	g_D2DRenderTarget->DrawTextLayout(D2D1::Point2F(left, y), layout, g_TextBrush);

	layout->Release();
}

void Hud::DrawPanel(float x, float y, float width, float height)
{
	if (!g_Ready) return;

	D2D1_ROUNDED_RECT panel = D2D1::RoundedRect(
		D2D1::RectF(x, y, x + width, y + height),
		6.0f, 6.0f);
	g_D2DRenderTarget->FillRoundedRectangle(panel, g_PanelBrush);
}