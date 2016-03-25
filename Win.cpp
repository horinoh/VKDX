#include "stdafx.h"

#include <iostream>

#include "Win.h"

Win::Win()
	: StdOut(nullptr)
	, StdErr(nullptr)
{
#ifdef _DEBUG
	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	SetConsoleTitle(TEXT("VKDX"));
	freopen_s(&StdOut, "CON", "w", stdout);
	freopen_s(&StdErr, "CON", "w", stderr);
	std::cout << White;
#endif
}
Win::~Win()
{
#ifdef _DEBUG
	SAFE_FCLOSE(StdOut);
	SAFE_FCLOSE(StdErr);
	FreeConsole();
#endif
}
