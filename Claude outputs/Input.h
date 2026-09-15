#pragma once
#include "main.h"

class Input
{
private:
    static BYTE m_OldKeyState[256];
    static BYTE m_PrevKeyState[256];

    // 追加
    static float m_MouseDeltaX;
    static float m_MouseDeltaY;

    // STEP36: タイトル/操作説明/設定画面など、マウス視点操作が要らない
    // 画面ではカーソルを隠したり中央に固定したりしないようにするための
    // フラグ。既定はfalse(=通常のカーソルが自由に使える状態)で、
    // 実際にマウス視点操作が要るGame中だけtrueにする
    // (Game.cpp/pauseMenu.cpp参照)。
    static bool m_MouseCaptureEnabled;

    // STEP37: 「クリックしても反応しない」対策 -- メニュー(タイトル/
    // 一時停止/設定/操作説明/リザルト)をマウスでも操作できるようにする。
    // キャプチャOFF中(=カーソルが自由なとき)のクライアント座標を毎フレーム
    // 保持しておき、各メニューのDrawUI()/Draw()と同じ座標系(SCREEN_WIDTH/
    // HEIGHTのピクセル)でヒットテストする。キャプチャON中(ゲームプレイの
    // 視点操作中)はカーソルが中央固定なので、この値は意味を持たない。
    static int m_MouseX;
    static int m_MouseY;
    static bool m_MouseLeftDown;
    static bool m_MouseLeftDownPrev;

public:
    static void Init();
    static void Uninit();
    static void Update();
    static bool GetKeyPress(BYTE KeyCode);
    static bool GetKeyTrigger(BYTE KeyCode);

    // 追加
    static float GetMouseDeltaX() { return m_MouseDeltaX; }
    static float GetMouseDeltaY() { return m_MouseDeltaY; }

    // STEP36
    static void SetMouseCaptureEnabled(bool enabled);
    static bool IsMouseCaptureEnabled() { return m_MouseCaptureEnabled; }

    // STEP37: メニューのマウス操作用。座標はクライアント領域のピクセル
    // (Hud::DrawText等と同じ原点・単位)。
    static int GetMouseX() { return m_MouseX; }
    static int GetMouseY() { return m_MouseY; }
    static bool GetMouseLeftTrigger() { return m_MouseLeftDown && !m_MouseLeftDownPrev; } // 押した瞬間だけtrue(GetKeyTrigger()と同じ考え方)
    static bool GetMouseLeftPress() { return m_MouseLeftDown; }
};