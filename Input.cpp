#include "main.h"
#include "Input.h"

BYTE Input::m_OldKeyState[256] = {};
BYTE Input::m_PrevKeyState[256] = {};

float Input::m_MouseDeltaX = 0.0f;
float Input::m_MouseDeltaY = 0.0f;

void Input::Init()
{
	memset(m_OldKeyState, 0, 256);
	memset(m_PrevKeyState, 0, 256);

	// hide the OS cursor for mouse-look
	ShowCursor(FALSE);
}
void Input::Uninit()
{
	ShowCursor(TRUE);
}
void Input::Update()
{
	memcpy(m_PrevKeyState, m_OldKeyState, 256);
	GetKeyboardState(m_OldKeyState);

	// mouse delta measured against the window center, then re-center
	// so the cursor never reaches the screen edge (FPS-style look)
	HWND hwnd = GetWindow();
	RECT rect;
	GetClientRect(hwnd, &rect);
	POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
	ClientToScreen(hwnd, &center);

	POINT curPos;
	GetCursorPos(&curPos);

	m_MouseDeltaX = (float)(curPos.x - center.x);
	m_MouseDeltaY = (float)(curPos.y - center.y);

	SetCursorPos(center.x, center.y);
}
bool Input::GetKeyPress(BYTE KeyCode)
{
	return (m_OldKeyState[KeyCode] & 0x80) != 0;
}
bool Input::GetKeyTrigger(BYTE KeyCode)
{
	return ((m_OldKeyState[KeyCode] & 0x80) != 0) && ((m_PrevKeyState[KeyCode] & 0x80) == 0);
}