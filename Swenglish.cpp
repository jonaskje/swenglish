#include <windows.h>

#pragma comment(linker, "/ALIGN:16")
#pragma comment(linker, "/MERGE:.rdata=.data")
#pragma comment(linker, "/MERGE:.text=.data")

bool g_enableSwedishChars = false;
bool g_lshift = false;
bool g_rshift = false;

LRESULT CALLBACK LowLevelKeyboardProc(
	_In_ int    nCode,
	_In_ WPARAM wParam,
	_In_ LPARAM lParam
	)
{
	if (nCode != HC_ACTION)
	{
		return CallNextHookEx(0, nCode, wParam, lParam);
	}

	bool keyDown = wParam == WM_KEYDOWN;
	bool keyUp = wParam == WM_KEYUP;

	KBDLLHOOKSTRUCT* p = (KBDLLHOOKSTRUCT*)lParam;

	// Use the windows button to enable swedish chars
	if (p->vkCode == 91 && !(p->flags & LLKHF_INJECTED)) {
		g_enableSwedishChars = keyDown;
		return 1;
	}

	if (p->vkCode == 160)
	{
		g_lshift = keyDown;
	}

	if (p->vkCode == 161)
	{
		g_rshift = keyDown;
	}

	if (g_enableSwedishChars)
	{
		WORD scan = 0;
		bool upperCase = g_lshift || g_rshift ||((GetKeyState(VK_CAPITAL) & 0x0001)!=0);

		if (p->scanCode == 26) // å
		{
			scan = upperCase ? L'Å' : L'å';
		}
		else if (p->scanCode == 40) // ä
		{
			scan = upperCase ? L'Ä' : L'ä';
		}
		else if (p->scanCode == 39) // ö
		{
			scan = upperCase ? L'Ö' : L'ö';
		}

		if (scan != 0)
		{
			INPUT newinput;
			newinput.type = INPUT_KEYBOARD;

			newinput.ki.wVk = 0;
			newinput.ki.wScan = scan;
			newinput.ki.dwFlags = KEYEVENTF_UNICODE | (keyUp ? KEYEVENTF_KEYUP : 0);
			newinput.ki.time = 0;
			newinput.ki.dwExtraInfo = 0;

			SendInput(1, &newinput, sizeof(INPUT));
			return 1;
		}
	}

	// Replace context menu button with the windows button
	if (p->vkCode == 93)
	{
		INPUT newinput;
		newinput.type = INPUT_KEYBOARD;

		newinput.ki.wVk = 91;
		newinput.ki.wScan = 91;
		newinput.ki.dwFlags = KEYEVENTF_EXTENDEDKEY | (keyUp ? KEYEVENTF_KEYUP : 0);
		newinput.ki.time = 0;
		newinput.ki.dwExtraInfo = 0;

		SendInput(1, &newinput, sizeof(INPUT));
		return 1;
	}

	return CallNextHookEx(0, nCode, wParam, lParam);
}

int __stdcall SwenglishMain()
{
	g_enableSwedishChars = false;
	g_lshift = false;
	g_rshift = false;

	if (SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, 0, 0))
	{
		MSG msg;
		while (GetMessage(&msg, 0, 0, 0));
		return 0;
	}
	else
	{
		return 1;
	}
}