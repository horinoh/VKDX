#include "stdafx.h"

#include <iostream>

#include "Win.h"

Win::Win()
	: StdOut(nullptr)
	, StdErr(nullptr)
{
#ifdef _DEBUG
	//!< ƒƒP[ƒ‹‚ð‹K’è‚É‚·‚é
	setlocale(LC_ALL, "");

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	SetConsoleTitle(TEXT("VKDX"));
	freopen_s(&StdOut, "CON", "w", stdout);
	freopen_s(&StdErr, "CON", "w", stderr);
	std::cout << White;
#endif

	__int64 Frequency;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&Frequency));
	SecondsPerCount = 1.0 / static_cast<double>(Frequency);
}
Win::~Win()
{
#ifdef _DEBUG
	SAFE_FCLOSE(StdOut);
	SAFE_FCLOSE(StdErr);
	FreeConsole();
#endif
}

void Win::OnCreate(HWND hWnd, HINSTANCE hInstance)
{
	GetClientRect(hWnd, &Rect);
	SetTimer(hWnd, NULL, 1000 / 60, nullptr);
}
void Win::OnSize(HWND hWnd, HINSTANCE hInstance)
{
	GetClientRect(hWnd, &Rect);
}
void Win::OnTimer(HWND hWnd, HINSTANCE hInstance)
{
	SendMessage(hWnd, WM_PAINT, 0, 0);
}
void Win::OnPaint(HWND hWnd, HINSTANCE hInstance)
{
}
void Win::OnDestroy(HWND hWnd, HINSTANCE hInstance)
{
}

void Win::ShowMessageBox(HWND hWnd, const std::string Message)
{
	MessageBox(hWnd, std::wstring(Message.begin(), Message.end()).c_str(), L"VERIFY_SUCCEEDED", MB_OK);
}
void Win::ShowMessageBoxW(HWND hWnd, const std::wstring Message)
{
	MessageBox(hWnd, Message.c_str(), L"VERIFY_SUCCEEDED", MB_OK);
}
