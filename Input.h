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

public:
    static void Init();
    static void Uninit();
    static void Update();
    static bool GetKeyPress(BYTE KeyCode);
    static bool GetKeyTrigger(BYTE KeyCode);

    // ’Ç‰Á
    static float GetMouseDeltaX() { return m_MouseDeltaX; }
    static float GetMouseDeltaY() { return m_MouseDeltaY; }
};