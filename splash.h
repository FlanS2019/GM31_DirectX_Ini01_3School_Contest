#pragma once
#include "Scene.h"

// STEP40: 起動時に一瞬だけ挟む「制作者ロゴ」スプラッシュ画面
// (logo_splash.png = "RUINS STUDIO")。3D背景は使わず、Renderer::Begin()の
// クリアカラー(不透明な黒 -- renderer.cpp参照)をそのまま背景として使う。
// SplashLogo::Update()がフェードイン→静止→フェードアウトのタイマーを
// 持ち、完了(またはスキップ)でManager::ChangeScene<Title>()を呼ぶ。
class Splash : public Scene
{
public:
	void Init() override;
	void Uninit() override;
	void Update() override {}
	void Draw() override {}
};
