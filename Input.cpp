#include "main.h"
#include "Input.h"

BYTE Input::m_OldKeyState[256] = {};
BYTE Input::m_PrevKeyState[256] = {};

// 追加
float Input::m_MouseDeltaX = 0.0f;
float Input::m_MouseDeltaY = 0.0f;

void Input::Init()
{
	memset(m_OldKeyState, 0, 256);
	memset(m_PrevKeyState, 0, 256);
}
void Input::Uninit()
{
}
void Input::Update()
{
    memcpy(m_PrevKeyState, m_OldKeyState, 256);
    GetKeyboardState(m_OldKeyState);

    // マウスの移動量を取得
    static POINT prevPos = {};
    POINT curPos;
    GetCursorPos(&curPos);

    m_MouseDeltaX = (float)(curPos.x - prevPos.x);
    m_MouseDeltaY = (float)(curPos.y - prevPos.y);

    //// マウスをウィンドウ中央に固定
    //HWND hwnd = GetActiveWindow();
    //RECT rect;
    //GetClientRect(hwnd, &rect);
    //POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
    //ClientToScreen(hwnd, &center);
    //SetCursorPos(center.x, center.y);
    //prevPos = center;
}
bool Input::GetKeyPress(BYTE KeyCode)
{
	return (m_OldKeyState[KeyCode] & 0x80) != 0;
}
bool Input::GetKeyTrigger(BYTE KeyCode)
{
	return ((m_OldKeyState[KeyCode] & 0x80) != 0) && ((m_PrevKeyState[KeyCode] & 0x80) == 0);
}