#pragma once

#ifndef VERIFY_SUCCEEDED
#define VERIFY_SUCCEEDED(vr) BREAK_ON_FAILED(vr)
//#define VERIFY_SUCCEEDED(vr) THROW_ON_FAILED(vr)
//#define VERIFY_SUCCEEDED(vr) MESSAGEBOX_ON_FAILED(vr)
#endif

//!< メモリリーク検出用
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#ifdef _DEBUG
#ifndef DEBUG_NEW 
#define DEBUG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
#define new DEBUG_NEW 
#endif 
#endif

#ifdef _DEBUG
#ifndef DEBUG_STDOUT
#define DEBUG_STDOUT
#endif
#ifndef DEBUG_OUTPUT
#define DEBUG_OUTPUT
#endif
#endif //!< _DEBUG

#include <iostream>
#include <ostream>
#include <vector>
#include <array>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <codecvt>
#include <functional>
#include <thread>
#include <bitset>

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if(nullptr != x) { delete x; x = nullptr; }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x) if(nullptr != x) { delete [] x; x = nullptr; }
#endif
#ifndef SAFE_FCLOSE
#define SAFE_FCLOSE(x) if(nullptr != x) { fclose(x); x = nullptr; }
#endif

#ifndef DEBUG_BREAK
#ifdef _DEBUG
#define DEBUG_BREAK() DebugBreak()
#else
#define DEBUG_BREAK()
#endif
#endif

#ifndef VERIFY
#ifdef  _DEBUG
//#define VERIFY(x) assert(x)
//#define VERIFY(x) if(!(x)) { DEBUG_BREAK(); }
#define VERIFY(x) if (!(x)) { throw std::runtime_error("VERIFY failed"); }
#else
#define VERIFY(x) (x)
#endif
#endif

class Win
{
public:
	Win();
	virtual ~Win();

	//virtual void OnInitialize(HWND hWnd, HINSTANCE hInstance) {}
	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance, LPCWSTR Title);
	virtual void OnSize(HWND hWnd, HINSTANCE hInstance);
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance);
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance);
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance);

	LONG GetClientRectWidth() const { return Rect.right - Rect.left; }
	LONG GetClientRectHeight() const { return Rect.bottom - Rect.top; }
	FLOAT GetAspectRatio(const FLOAT Width, const FLOAT Height) const { return Width / Height; }
	FLOAT GetAspectRatioOfClientRect() const { return GetAspectRatio(static_cast<FLOAT>(GetClientRectWidth()), static_cast<FLOAT>(GetClientRectHeight())); }

	static std::string ToString(const std::wstring& WStr) {
#if 1
		const auto Size = WideCharToMultiByte(CP_UTF8, 0, WStr.c_str(), -1, nullptr, 0, nullptr, nullptr);
		auto Buffer = new CHAR[Size];
		WideCharToMultiByte(CP_UTF8, 0, WStr.c_str(), -1, Buffer, Size, nullptr, nullptr);
		const auto Str = std::string(Buffer, Buffer + Size - 1);
		delete[] Buffer;
		return Str;
#elif 0
		const auto Size = lstrlen(WStr.c_str()) + 1;
		auto Buffer = new char[Size];
		size_t i;
		wcstombs_s(&i, Buffer, Size, WStr.c_str(), _TRUNCATE);
		const auto Str = std::string(Buffer, Buffer + Size - 1);
		delete[] Buffer;
		return Str;
#else
		//!< VS2019 : C4244
		return std::string(WStr.begin(), WStr.end());
#endif
	}
	static std::wstring ToWString(const std::string& Str) {
#if 0
		const auto Size = MultiByteToWideChar(CP_UTF8, 0, Str.c_str(), -1, nullptr, 0);
		auto Buffer = new wchar_t[Size];
		MultiByteToWideChar(CP_UTF8, 0, Str.c_str(), -1, Buffer, Size);
		const auto WStr = std::wstring(Buffer, Buffer + Size - 1);
		delete[] Buffer;
		return WStr;
#elif 0
		const auto Size = strlen(Str.c_str()) + 1;
		auto Buffer = new wchar_t[Size];
		size_t i;
		mbstowcs_s(&i, Buffer, Size, Str.c_str(), _TRUNCATE);
		const auto WStr = std::wstring(Buffer, Buffer + Size - 1);
		delete[] Buffer;
		return WStr;
#else
		return std::wstring(Str.begin(), Str.end());
#endif
	}
	static std::wstring ToWString(const char* Str) { return std::wstring(&Str[0], &Str[strlen(Str)]); }

	virtual const std::wstring& GetTitleW() const { return TitleW; }
	virtual std::string GetTitle() const { return ToString(TitleW); }
	void SetTitleW(LPCWSTR Title) { TitleW = Title; }

	std::wstring GetBasePath() const { return TEXT(".\\") + GetTitleW(); }

	static void ShowMessageBox(HWND hWnd, const char* Str) { MessageBox(hWnd, ToWString(Str).c_str(), TEXT("CAPTION"), MB_OK); }
#ifdef DEBUG_STDOUT
	static void SetColor(const WORD Color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE) { 
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Color | FOREGROUND_INTENSITY);
	}
#endif
	enum class LogType : unsigned char
	{
		Log,
		Warning,
		Error,
	};
private:
	template <typename T> static void _Log(const LogType Type, T Str);
	template <typename T> static void _Logf(const LogType Type, T Format, ...);
public:
	template <typename T> static void Log(const T Str) { _Log(LogType::Log, Str); }
	template <typename T> static void Warning(const T Str) { _Log(LogType::Warning, Str); }
	template <typename T> static void Error(const T Str) { _Log(LogType::Error, Str); }
	template <typename ... T> static void Logf(const char *Format, T const & ... Args) { _Logf(LogType::Log, Format, Args ...); }
	template <typename ... T> static void Logf(const WCHAR *Format, T const & ... Args) { _Logf(LogType::Log, Format, Args ...); }
	template <typename ... T> static void Warningf(const char *Format, T const & ... Args) { _Logf(LogType::Warning, Format, Args ...); }
	template <typename ... T> static void Warningf(const WCHAR *Format, T const & ... Args) { _Logf(LogType::Warning, Format, Args ...); }
	template <typename ... T> static void Errorf(const char *Format, T const & ... Args) { _Logf(LogType::Error, Format, Args ...); }
	template <typename ... T> static void Errorf(const WCHAR *Format, T const & ... Args) { _Logf(LogType::Error, Format, Args ...); }
	template <typename T> static void LogOK(T Str);
	template <typename T> static void LogNG(T Str);

protected:
	RECT Rect;
	std::wstring TitleW;

#ifdef DEBUG_STDOUT
private:
	FILE* StdOut = nullptr; 
	FILE* StdErr = nullptr;
#endif
};

#ifdef _DEBUG
class PerformanceCounter
{
public:
	PerformanceCounter(const std::string& Label = "");
	~PerformanceCounter();
private:
	double SecondsPerCount = 0.0;
	__int64 Start;
	std::string Label;
};
#endif

#ifdef DEBUG_STDOUT
static std::ostream& Red(std::ostream& rhs) { Win::SetColor(FOREGROUND_RED); return rhs; }
static std::ostream& Green(std::ostream& rhs) { Win::SetColor(FOREGROUND_GREEN); return rhs; }
static std::ostream& Blue(std::ostream& rhs) { Win::SetColor(FOREGROUND_BLUE); return rhs; }
static std::ostream& Yellow(std::ostream& rhs) { Win::SetColor(FOREGROUND_RED | FOREGROUND_GREEN); return rhs; }
static std::ostream& Purple(std::ostream& rhs) { Win::SetColor(FOREGROUND_RED | FOREGROUND_BLUE); return rhs; }
static std::ostream& Lightblue(std::ostream& rhs) { Win::SetColor(FOREGROUND_GREEN | FOREGROUND_BLUE); return rhs; }
static std::ostream& White(std::ostream& rhs) { Win::SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); return rhs; }

static std::wostream& Red(std::wostream& rhs) { Win::SetColor(FOREGROUND_RED); return rhs; }
static std::wostream& Green(std::wostream& rhs) { Win::SetColor(FOREGROUND_GREEN); return rhs; }
static std::wostream& Blue(std::wostream& rhs) { Win::SetColor(FOREGROUND_BLUE); return rhs; }
static std::wostream& Yellow(std::wostream& rhs) { Win::SetColor(FOREGROUND_RED | FOREGROUND_GREEN); return rhs; }
static std::wostream& Purple(std::wostream& rhs) { Win::SetColor(FOREGROUND_RED | FOREGROUND_BLUE); return rhs; }
static std::wostream& Lightblue(std::wostream& rhs) { Win::SetColor(FOREGROUND_GREEN | FOREGROUND_BLUE); return rhs; }
static std::wostream& White(std::wostream& rhs) { Win::SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); return rhs; }

#define COUT_OK White << " [ " << Green << "OK" << White << " ]"
#define COUT_NG White << " [ " << Red << "NG" << White << " ]"
#endif