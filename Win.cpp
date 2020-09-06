#include "Win.h"

#if 0
//!< pair, tuple
std::pair<int, float> getTwo() { return { 1, 1.0f }; }
std::tuple<int, float, double> getThree() { return{ 1, 1.0f, 1.0 }; }
auto [x, y] = getTwo();
auto [a, b, c] = getThree();
#endif

#if 0
//!< optional : �����l (�ʓrbool�l�Ǘ����Ă���P�[�X���ŗL��)
std::optional<int> a;		//!< ���ݒ�
std::optional<int> b = 1;	//!< �ݒ��
-1 == a.value_or(-1);		//!< ���ݒ�Ȃ̂�-1
1 == b.value_or(-1);		//!< �ݒ�ςȂ̂�1
if (b) { b.reset(); }		//!< �ݒ�ςȂ疢�ݒ��
-1 == b.value_or(-1);		//!< ���ݒ�Ȃ̂�-1
#endif

#if 0
//!< variant : union���
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
	//!< ���P�[�����K��ɂ���
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

void Win::OnCreate(HWND hWnd, HINSTANCE /*hInstance*/, LPCWSTR Title)
{
	SetTitleW(Title);

	GetClientRect(hWnd, &Rect);
}
void Win::OnExitSizeMove(HWND hWnd, HINSTANCE /*hInstance*/)
{
	GetClientRect(hWnd, &Rect);
}
void Win::OnTimer(HWND hWnd, HINSTANCE /*hInstance*/)
{
	SendMessage(hWnd, WM_PAINT, 0, 0);
}
void Win::OnPaint(HWND /*hWnd*/, HINSTANCE /*hInstance*/)
{
}
void Win::OnDestroy(HWND /*hWnd*/, HINSTANCE /*hInstance*/)
{
}

template<> void Win::_Log(
#ifdef DEBUG_STDOUT
	const LogType Type,
#else
	const LogType /*Type*/,
#endif
#if defined(DEBUG_STDOUT) || defined(DEBUG_OUTPUT)
	const char* Str)
#else
	const char* /*Str*/)
#endif
{
#ifdef DEBUG_STDOUT
	//!< �W���o�́A�G���[�o�͂�
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
	//!< Output�E�C���h�E��
	OutputDebugString(ToWString(Str).c_str());
#endif	
}
template<> void Win::_Log(
#ifdef DEBUG_STDOUT
	const LogType Type,
#else
	const LogType /*Type*/,
#endif
#if defined(DEBUG_STDOUT) || defined(DEBUG_OUTPUT)
	const WCHAR* Str)
#else
	const WCHAR* /*Str*/)
#endif
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
#if defined(DEBUG_STDOUT) || defined(DEBUG_OUTPUT)
template<> static void Win::LogOK(const char* Str)
#else
template<> static void Win::LogOK(const char* /*Str*/)
#endif
{
#ifdef DEBUG_STDOUT
	std::cout << Str << COUT_OK << std::endl;
#endif
#ifdef DEBUG_OUTPUT
	OutputDebugString(ToWString(Str).c_str());
	OutputDebugString(TEXT("[ OK ]\n"));
#endif	
}

#if defined(DEBUG_STDOUT) || defined(DEBUG_OUTPUT)
template<> static void Win::LogOK(const WCHAR* Str)
#else
template<> static void Win::LogOK(const WCHAR* /*Str*/)
#endif
{
#ifdef DEBUG_STDOUT
	std::wcout << Str << COUT_OK << std::endl;
#endif
#ifdef DEBUG_OUTPUT
	OutputDebugString(Str); OutputDebugString(TEXT("[ OK ]\n"));
#endif	
}
#if defined(DEBUG_STDOUT) || defined(DEBUG_OUTPUT)
template<> static void Win::LogNG(const char* Str)
#else
template<> static void Win::LogNG(const char* /*Str*/)
#endif
{
#ifdef DEBUG_STDOUT
	std::cerr << Str << COUT_NG << std::endl;
#endif
#ifdef DEBUG_OUTPUT
	OutputDebugString(ToWString(Str).c_str());
	OutputDebugString(TEXT("[ NG ]\n"));
#endif
}
#if defined(DEBUG_STDOUT) || defined(DEBUG_OUTPUT)
template<> static void Win::LogNG(const WCHAR* Str)
#else
template<> static void Win::LogNG(const WCHAR* /*Str*/)
#endif
{
#ifdef DEBUG_STDOUT
	std::wcerr << Str << COUT_NG << std::endl;
#endif
#ifdef DEBUG_OUTPUT
	OutputDebugString(Str); OutputDebugString(TEXT("[ NG ]\n"));
#endif
}

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
#ifdef DEBUG_STDOUT
	std::cout << Label << " : " << (End - Start) * SecondsPerCount * 1000.0 << " msec" << std::endl << std::endl;
#endif
}
