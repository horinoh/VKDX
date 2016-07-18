#pragma once

//!< ƒƒ‚ƒŠƒŠ[ƒNŒŸo
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#ifdef _DEBUG
#ifndef DBG_NEW 
#define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ ) 
#define new DBG_NEW 
#endif 
#endif

#include <ostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <iostream>
#include <sstream>
#include <codecvt> 

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if(nullptr != x) { delete x; x = nullptr; }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x) if(nullptr != x) { delete [] x; x = nullptr; }
#endif
#ifndef SAFE_FCLOSE
#define SAFE_FCLOSE(x) if(nullptr != x) { fclose(x); x = nullptr; }
#endif
#ifndef VERIFY
#ifdef  _DEBUG
//#define VERIFY(x) assert(x)
#define VERIFY(x) if (!(x)) { throw std::runtime_error("VERIFY failed"); }
#else
#define VERIFY(x) (x)
#endif
#endif
#ifndef SHADER_PATH
#define SHADER_PATH L".\\"
#endif

class Win
{
public:
	Win();
	virtual ~Win();

	//virtual void OnInitialize(HWND hWnd, HINSTANCE hInstance) {}
	virtual void OnCreate(HWND hWnd, HINSTANCE hInstance);
	virtual void OnSize(HWND hWnd, HINSTANCE hInstance);
	virtual void OnTimer(HWND hWnd, HINSTANCE hInstance);
	virtual void OnPaint(HWND hWnd, HINSTANCE hInstance);
	virtual void OnDestroy(HWND hWnd, HINSTANCE hInstance);

	static void SetColor(const WORD Color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE) {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Color | FOREGROUND_INTENSITY);
	}

	LONG GetClientRectWidth() const { return Rect.right - Rect.left; }
	LONG GetClientRectHeight() const { return Rect.bottom - Rect.top; }

	static void ShowMessageBox(HWND hWnd, const std::string Message);
	static void ShowMessageBoxW(HWND hWnd, const std::wstring Message);

protected:
	RECT Rect;

private:
	FILE* StdOut;
	FILE* StdErr;
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
