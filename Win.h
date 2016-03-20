#pragma once

#include <ostream>

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if(nullptr != x) { delete x; x = nullptr; }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x) if(nullptr != x) { delete [] x; x = nullptr; }
#endif
#ifndef SAFE_FCLOSE
#define SAFE_FCLOSE(x) if(nullptr != x) { fclose(x); x = nullptr; }
#endif

class Win
{
public:
	Win();
	virtual ~Win();

	virtual void OnCreate(HWND hWnd) {}
	virtual void OnSize(HWND hWnd) {}
	virtual void OnTimer(HWND hWnd) {}
	virtual void OnPaint(HWND hWnd) {}
	virtual void OnDestroy(HWND hWnd) {}

	static void SetColor(const WORD Color = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE) {
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), Color | FOREGROUND_INTENSITY);
	}
private:
	FILE* StdOut;
	FILE* StdErr;
};

static std::ostream& Red(std::ostream& rhs) { Win::SetColor(FOREGROUND_RED); return rhs; }
static std::ostream& Green(std::ostream& rhs) { Win::SetColor(FOREGROUND_GREEN); return rhs; }
static std::ostream& Blue(std::ostream& rhs) { Win::SetColor(FOREGROUND_BLUE); return rhs; }
static std::ostream& Yellow(std::ostream& rhs) { Win::SetColor(FOREGROUND_RED | FOREGROUND_GREEN); return rhs; }
static std::ostream& Purple(std::ostream& rhs) { Win::SetColor(FOREGROUND_RED | FOREGROUND_BLUE); return rhs; }
static std::ostream& Lightblue(std::ostream& rhs) { Win::SetColor(FOREGROUND_GREEN | FOREGROUND_BLUE); return rhs; }
static std::ostream& White(std::ostream& rhs) { Win::SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); return rhs; }