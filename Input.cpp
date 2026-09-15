#include "main.h"
#include "Input.h"

BYTE Input::m_OldKeyState[256] = {};
BYTE Input::m_PrevKeyState[256] = {};

float Input::m_MouseDeltaX = 0.0f;
float Input::m_MouseDeltaY = 0.0f;
bool Input::m_MouseCaptureEnabled = false;

int Input::m_MouseX = 0;
int Input::m_MouseY = 0;
bool Input::m_MouseLeftDown = false;
bool Input::m_MouseLeftDownPrev = false;

void Input::Init()
{
	memset(m_OldKeyState, 0, 256);
	memset(m_PrevKeyState, 0, 256);

	m_MouseCaptureEnabled = false;
}
void Input::Uninit()
{
	ShowCursor(TRUE); 
}
void Input::SetMouseCaptureEnabled(bool enabled)
{
	if (enabled == m_MouseCaptureEnabled) return;
	m_MouseCaptureEnabled = enabled;

	HWND hwnd = GetWindow();

	if (enabled)
	{
		RECT rect;
		GetClientRect(hwnd, &rect);
		POINT center = { (rect.right - rect.left) / 2, (rect.bottom - rect.top) / 2 };
		ClientToScreen(hwnd, &center);
		SetCursorPos(center.x, center.y);

		ShowCursor(FALSE);
	}
	else
	{
		ShowCursor(TRUE);
	}

	m_MouseDeltaX = 0.0f;
	m_MouseDeltaY = 0.0f;
}
void Input::Update()
{
	memcpy(m_PrevKeyState, m_OldKeyState, 256);
	GetKeyboardState(m_OldKeyState);

	{
		HWND hwnd = GetWindow();

		POINT p;
		GetCursorPos(&p);
		ScreenToClient(hwnd, &p);
		m_MouseX = p.x;
		m_MouseY = p.y;

		m_MouseLeftDownPrev = m_MouseLeftDown;
		m_MouseLeftDown = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
	}

	if (!m_MouseCaptureEnabled)
	{
		m_MouseDeltaX = 0.0f;
		m_MouseDeltaY = 0.0f;
		return;
	}

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