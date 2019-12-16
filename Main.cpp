#include <Windows.h>
#include <stdio.h>
#include <conio.h>

BOOL   gShouldStop = FALSE;

//BYTE   GKeyCode    = VK_HOME;

//BYTE   GScanCode   = NULL;

HANDLE gMutex      = NULL;

DWORD WINAPI Interrupt(LPVOID)
{
	int key = 0;

	key = _getch();

	wprintf_s(L"Exiting...\n");

	gShouldStop = TRUE;

	return(0);
}

BOOL CALLBACK EnumRDPWindowsProc(HWND Window, LPARAM)
{
	wchar_t ClassName[128]   = { 0 };

	wchar_t WindowTitle[128] = { 0 };

	if (GetClassName(Window, ClassName, sizeof(ClassName) / sizeof(wchar_t)) == 0)
	{
		wprintf_s(L"ERROR: GetClassName failed!\n");

		return FALSE;
	}

	GetWindowText(Window, WindowTitle, sizeof(WindowTitle) / sizeof(wchar_t));

	if (wcslen(ClassName) == 0)
	{
		_snwprintf_s(ClassName, sizeof(ClassName), L"[NoClass]\n");
	}

	if (wcslen(WindowTitle) == 0)
	{
		_snwprintf_s(WindowTitle, sizeof(WindowTitle), L"[NoTitle]\n");
	}	

	if (wcscmp(ClassName, L"TscShellContainerClass") == 0)
	{
		HWND OriginalForegroundWindow = GetForegroundWindow();

		HWND WindowHandle = FindWindowEx(NULL, NULL, ClassName, WindowTitle);
		
		if (WindowHandle != NULL)
		{
			//wprintf_s(L"[%I64d] Found %s - %s\n", GetTickCount64(), ClassName, WindowTitle);

			SetForegroundWindow(WindowHandle);

			INPUT Input = { 0 };

			POINT CurrentPosition = { 0 };

			GetCursorPos(&CurrentPosition);

			Input.type = INPUT_MOUSE;

			Input.mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

			//Input.mi.dx = (CurrentPosition.x + 1) * (65536.0f / GetSystemMetrics(SM_CXSCREEN));

			//Input.mi.dy = (CurrentPosition.y + 1) * (65536.0f / GetSystemMetrics(SM_CYSCREEN));

			//SendInput(1, &Input, sizeof(INPUT));

			Input.mi.dx = (CurrentPosition.x) * (65536.0f / GetSystemMetrics(SM_CXSCREEN));
			
			Input.mi.dy = (CurrentPosition.y) * (65536.0f / GetSystemMetrics(SM_CYSCREEN));

			SendInput(1, &Input, sizeof(INPUT));
			
			SetForegroundWindow(OriginalForegroundWindow);
		}
	}

	return TRUE;
}

int wmain(int, wchar_t *)
{
	wprintf_s(L"ImAlive - Joseph Ryan Ries, (c) 2015\n");

	wprintf_s(L"Simulating RDP activity.\nPress any key to stop...\n\n");

	gMutex = CreateMutex(NULL, FALSE, L"ImAliveApp");

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		MessageBox(NULL, L"An instance of the program is already running.", L"Error", MB_OK | MB_ICONERROR);

		return(0);
	}

	if (CreateThread(NULL, 0, Interrupt, NULL, 0, NULL) == NULL)
	{
		wprintf_s(L"ERROR: Could not create interrupt thread! GetLastError: 0x%x\n", GetLastError());

		return(0);
	}

	while (gShouldStop == FALSE)
	{
		int Rest = 0;

		if (EnumWindows(EnumRDPWindowsProc, NULL) == FALSE)
		{
			wprintf_s(L"ERROR: EnumWindows returned false! GetLastError: 0x%x\n", GetLastError());
		}

		while (Rest < 60)
		{
			if (gShouldStop)
			{
				break;
			}

			Sleep(1000);

			Rest++;
		}
	}

	return(0);
}