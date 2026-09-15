#pragma once
#include "main.h"

class Input
{
private:
    static BYTE m_OldKeyState[256];
    static BYTE m_PrevKeyState[256];

    // ’Ç‰Á
    static float m_MouseDeltaX;
    static float m_MouseDeltaY;

    static bool m_MouseCaptureEnabled;

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

    static float GetMouseDeltaX() { return m_MouseDeltaX; }
    static float GetMouseDeltaY() { return m_MouseDeltaY; }

    static void SetMouseCaptureEnabled(bool enabled);
    static bool IsMouseCaptureEnabled() { return m_MouseCaptureEnabled; }

    static int GetMouseX() { return m_MouseX; }
    static int GetMouseY() { return m_MouseY; }
    static bool GetMouseLeftTrigger() { return m_MouseLeftDown && !m_MouseLeftDownPrev; }
    static bool GetMouseLeftPress() { return m_MouseLeftDown; }
};