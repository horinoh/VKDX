#include "stdafx.h"

#include "Win.h"

Win::Win()
{
#ifdef DEBUG_STDOUT
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
}
Win::~Win()
{
#ifdef DEBUG_STDOUT
	SAFE_FCLOSE(StdOut);
	SAFE_FCLOSE(StdErr);
	FreeConsole();
#endif
}

void Win::OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title)
{
	SetTitleW(Title);

	GetClientRect(hWnd, &Rect);

	//SetTimer(hWnd, NULL, 1000 / 60, nullptr);
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

void Win::Log(const LogType Type, const char* Str)
{
#ifdef DEBUG_STDOUT
	switch (Type)
	{
	default: break;
	case LogType::Log: 
		std::cout << Str;
		break;
	case LogType::Warning: 
		std::cout << Yellow << Str << White; 
		break;
	case LogType::Error: 
		std::cerr << Red << Str << White; 
		break;
	}
#endif
#ifdef DEBUG_OUTPUT
	OutputDebugString(std::wstring(&Str[0], &Str[strlen(Str)]).c_str());
#endif	
}
void Win::Logf(const LogType Type, const char* Format, ...)
{
	va_list List;
	va_start(List, Format);
	static char Buffer[1024];
	const auto Count = vsnprintf(Buffer, sizeof(Buffer), Format, List);
	if (Count) {
		Log(Type, std::string(&Buffer[0], &Buffer[Count]).c_str());
	}
	va_end(List);
}

void Win::LogOK(const char* Str)
{
#ifdef DEBUG_STDOUT
	std::cout << Str << COUT_OK << std::endl;
#endif
#ifdef DEBUG_OUTPUT
	OutputDebugString(std::wstring(&Str[0], &Str[strlen(Str)]).c_str());
	OutputDebugString(TEXT("[ OK ]\n"));
#endif	
}
void Win::LogNG(const char* Str)
{
#ifdef DEBUG_STDOUT
	std::cerr << Str << COUT_NG << std::endl;
#endif
#ifdef DEBUG_OUTPUT
	OutputDebugString(std::wstring(&Str[0], &Str[strlen(Str)]).c_str());
	OutputDebugString(TEXT("[ NG ]\n"));
#endif
}


#ifdef _DEBUG
PerformanceCounter::PerformanceCounter(const std::string& Label)
	: Label(Label)
{
	__int64 Frequency;
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&Frequency));
	SecondsPerCount = 1.0 / static_cast<double>(Frequency);

	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&Start));
}
PerformanceCounter::~PerformanceCounter() 
{
	__int64 End;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&End));
	std::cout << Label << (End - Start) * SecondsPerCount * 1000.0 << " msec" << std::endl << std::endl;
}
#endif