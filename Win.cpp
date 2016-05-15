#include "stdafx.h"

#include <iostream>

#include "Win.h"

Win::Win()
	: StdOut(nullptr)
	, StdErr(nullptr)
{
#ifdef _DEBUG
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
