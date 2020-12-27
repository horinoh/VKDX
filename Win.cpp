#include "Win.h"

#if 0
//!< pair, tuple
std::pair<int, float> getTwo() { return { 1, 1.0f }; }
std::tuple<int, float, double> getThree() { return{ 1, 1.0f, 1.0 }; }
auto [x, y] = getTwo();
auto [a, b, c] = getThree();
#endif

#if 0
//!< optional : 無効値 (別途bool値管理しているケース等で有効)
std::optional<int> a;		//!< 未設定
std::optional<int> b = 1;	//!< 設定済
-1 == a.value_or(-1);		//!< 未設定なので-1
1 == b.value_or(-1);		//!< 設定済なので1
if (b) { b.reset(); }		//!< 設定済なら未設定へ
-1 == b.value_or(-1);		//!< 未設定なので-1
#endif

#if 0
//!< variant : union代替
std::variant<int, float, bool> a = 1, b = 3.14f, c = true;
0 == a.index();
1 == b.index();
2 == c.index();
3.14f == get<1>(b)
if (std::holds_alternative<bool>(c)) { true == get<bool>(c); }
#endif

Win::Win()
{
#ifdef DEBUG_STDOUT
	//!< ロケールを規定にする
	setlocale(LC_ALL, "");

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	AllocConsole();
	AttachConsole(GetCurrentProcessId());
	SetConsoleTitle(TEXT("VKDX"));
	freopen_s(&StdOut, "CON", "w", stdout);
	freopen_s(&StdErr, "CON", "w", stderr);
	std::cout << White;

//#ifdef __cplusplus
//	std::cout << "__cplusplus = " << __cplusplus << std::endl;
//#endif

#ifdef _MSVC_LANG
	std::cout << "_MSVC_LANG = " << _MSVC_LANG;
#if 201703L < _MSVC_LANG
	std::cout << ", Latest" << std::endl; 
#elif 201402L < _MSVC_LANG
	std::cout << ", C++17" << std::endl;
#else
	std::cout << ", C++14" << std::endl;
#endif
#endif

#endif //!< DEBUG_STDOUT
}
Win::~Win()
{
#ifdef DEBUG_STDOUT
	SAFE_FCLOSE(StdOut);
	SAFE_FCLOSE(StdErr);
	FreeConsole();
#endif
}

void Win::OnCreate(HWND hWnd, [[maybe_unused]] HINSTANCE hInstance, LPCWSTR Title)
{
	SetTitleW(Title);

	GetClientRect(hWnd, &Rect);
}
void Win::OnExitSizeMove(HWND hWnd, [[maybe_unused]] HINSTANCE hInstance)
{
	GetClientRect(hWnd, &Rect);
}
void Win::OnTimer(HWND hWnd, [[maybe_unused]] HINSTANCE hInstance)
{
	SendMessage(hWnd, WM_PAINT, 0, 0);
}
void Win::OnPaint([[maybe_unused]] HWND hWnd, [[maybe_unused]] HINSTANCE hInstance)
{
}
void Win::OnDestroy([[maybe_unused]] HWND hWnd, [[maybe_unused]] HINSTANCE hInstance)
{
}

template<> void Win::_Log([[maybe_unused]] const LogType Type, [[maybe_unused]] const char* Str)
{
#ifdef DEBUG_STDOUT
	//!< 標準出力、エラー出力へ
	switch (Type)
	{
		using enum LogType;
	default: break;
	case Log:
		std::cout << Str;
		break;
	case Warning:
		std::cout << Yellow << Str << White;
		break;
	case Error:
		std::cerr << Red << Str << White;
		break;
	}
#endif
#ifdef DEBUG_OUTPUT
	//!< Outputウインドウへ
	OutputDebugString(data(ToWString(Str)));
#endif	
}
template<> void Win::_Log([[maybe_unused]] const LogType Type, [[maybe_unused]] const WCHAR* Str)
{
#ifdef DEBUG_STDOUT
	switch (Type)
	{
	default: break;
	case LogType::Log:
		std::wcout << Str;
		break;
	case LogType::Warning:
		std::wcout << Yellow << Str << White;
		break;
	case LogType::Error:
		std::wcerr << Red << Str << White;
		break;
	}
#endif
#ifdef DEBUG_OUTPUT
	OutputDebugString(Str);
#endif	
}
template<> void Win::_Logf(const LogType Type, const char* Format, ...)
{
	va_list List;
	va_start(List, Format);
	static char Buffer[1024];
	const auto Count = vsnprintf(Buffer, sizeof(Buffer), Format, List);
	if (Count) {
		_Log<const char*>(Type, Buffer);
	}
	va_end(List);
}
template<> void Win::_Logf(const LogType Type, const WCHAR* Format, ...)
{
	va_list List;
	va_start(List, Format);
	static WCHAR Buffer[1024];
	const auto Count = vswprintf(Buffer, sizeof(Buffer), Format, List);
	if (Count) {
		_Log<const WCHAR*>(Type, Buffer);
	}
	va_end(List);
}
template<> static void Win::LogOK([[maybe_unused]] const char* Str)
{
#ifdef DEBUG_STDOUT
	std::cout << Str << COUT_OK << std::endl;
#endif
#ifdef DEBUG_OUTPUT
	OutputDebugString(data(ToWString(Str)));
	OutputDebugString(TEXT("[ OK ]\n"));
#endif	
}

template<> static void Win::LogOK([[maybe_unused]] const WCHAR* Str)
{
#ifdef DEBUG_STDOUT
	std::wcout << Str << COUT_OK << std::endl;
#endif
#ifdef DEBUG_OUTPUT
	OutputDebugString(Str); 
	OutputDebugString(TEXT("[ OK ]\n"));
#endif	
}
template<> static void Win::LogNG([[maybe_unused]] const char* Str)
{
#ifdef DEBUG_STDOUT
	std::cerr << Str << COUT_NG << std::endl;
#endif
#ifdef DEBUG_OUTPUT
	OutputDebugString(data(ToWString(Str)));
	OutputDebugString(TEXT("[ NG ]\n"));
#endif
}
template<> static void Win::LogNG([[maybe_unused]] const WCHAR* Str)
{
#ifdef DEBUG_STDOUT
	std::wcerr << Str << COUT_NG << std::endl;
#endif
#ifdef DEBUG_OUTPUT
	OutputDebugString(Str); 
	OutputDebugString(TEXT("[ NG ]\n"));
#endif
}

PerformanceCounter::PerformanceCounter(std::string_view Label)
	: Label(Label)
{
	if (QueryPerformanceCounter(&Start)) {
	}
}
PerformanceCounter::~PerformanceCounter() 
{
	LARGE_INTEGER End;
	if (QueryPerformanceCounter(&End)) {
		LARGE_INTEGER Frequency;
		if (QueryPerformanceFrequency(&Frequency)) {
#ifdef DEBUG_STDOUT
			std::cout << Label << " : " << (End.QuadPart - Start.QuadPart) * 1000 / Frequency.QuadPart << " msec" << std::endl << std::endl;
#endif
		}
	}
}
