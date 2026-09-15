#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

#define NOMINMAX
#include <windows.h>
#include <assert.h>
#include <functional>

#include <list>
#include <vector>

#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")


#include <DirectXMath.h>
using namespace DirectX;

#include "DirectXTex.h"
#if _DEBUG
#pragma comment (lib, "DirectXTex_Debug.lib")
#else
#pragma comment (lib, "DirectXTex.lib")
#endif

#include "vector3.h"

#pragma comment (lib, "winmm.lib")


// STEP34: 1280x720 -> 1920x1080(x1.5)。既存UIの絶対座標は全部1.5倍
// してレイアウトを保ったまま拡大してある(各画面のcpp参照)。
#define SCREEN_WIDTH	(1920)
#define SCREEN_HEIGHT	(1080)


HWND GetWindow();

void Invoke(std::function<void()> Function, int Time);

