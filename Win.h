#pragma once

#pragma warning(disable: 4505) //!< unreferenced function has been removed

//!< メモリリーク検出用
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>
#include <filesystem>
#include <source_location>

#ifdef _DEBUG
#ifndef DEBUG_NEW 
//#define DEBUG_NEW new (_NORMAL_BLOCK , __FILE__, __LINE__) 
#define DEBUG_NEW new (_NORMAL_BLOCK , std::source_location::current().file_name(), std::source_location::current().line()) 
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

//!< HDR環境が無いので保留…
//!< DXの場合「HDRのゲームとアプリを使用する」設定が必要 (Need to enable "Play HDR game and apps" in windows settings )
//!< https://support.microsoft.com/ja-jp/help/4040263/windows-10-hdr-advanced-color-settings
//#define USE_HDR
//#define USE_FULLSCREEN
#define USE_DEPTH //!< [ ToonDX, ToonVK ], [ RenderTargetDX, RenderTargetVK ]
#define USE_SCREENSPACE_WIREFRAME //!< [ ToonDX, ToonVK ]
#define USE_INDIRECT //!< [ FullscreenDX, FullscreenVK, RTTriangleDX, RTTriangleVK ]
#define USE_DISTANCE_FUNCTION //!< [ FullscreenDX, FullscreenVK ]
#define USE_PIPELINE_SERIALIZE //!< [ *DX, *VK ]
//#define USE_PARALLAX_MAP //!< [ NormalMapDX, NormalMapVK ]
//#define USE_SKY_DOME //!< [ CubeMapDX, CubeMapVK ]
#define USE_GBUFFER_VISUALIZE //!< [ DeferredDX, DeferredVK ]
#define USE_SHADOWMAP_VISUALIZE //!< [ ShadowMapDX, ShadowMapVK ]
#define USE_SHADER_REFLECTION //!< [ TriangleDX ]
#define USE_BLAS_COMPACTION //!< [ RTTriangleDX, RTTriangleVK ] ... TLAS もコンパクションできるが、効果が大きいのは BLAS なので、ここでは BLAS のみにしている
//#define USE_CONVEXHULL //!< [ DracoDX, DracoVK ]

//!< Property - C/C++ - Command Line - Additional Options に "/await" と記述する必要がある (Need to write "/await" to Property - C/C++ - Command Line - Additional Options)
#define USE_EXPERIMENTAL

#define ALWAYS_REBUILD_PIPELINE

#include <ranges>
#include <algorithm>
#include <bitset>
#include <variant>
#include <cassert>
#include <functional>
#include <thread>
#include <numeric>
#include <numbers>
#include <optional>
#include <random>

#include <iostream>
#include <ostream>
#include <sstream>
//#include <format>

#include <codecvt>
#include <charconv>
#include <locale>
#include <string>
#include <stack>

#include <array>
#include <fstream>

#ifdef USE_EXPERIMENTAL
#include <experimental/coroutine>
#else
//#include <coroutine>
#endif

//#include <cinttypes>

#include <dwmapi.h>
//!< GET_X_LPARAM(), GET_Y_LPARAM() に必要
#include <windowsx.h>

#ifndef SAFE_FCLOSE
#define SAFE_FCLOSE(x) if(nullptr != x) { fclose(x); x = nullptr; }
#endif

#ifndef DEBUG_BREAK
#ifdef _DEBUG
#define DEBUG_BREAK() __debugbreak()
#else
#define DEBUG_BREAK()
#endif
#endif

#ifndef LOG_OK
//#define LOG_OK() LogOK(__func__)
#define LOG_OK() LogOK(std::source_location::current().function_name())
#endif

#ifndef PERFORMANCE_COUNTER
//#define PERFORMANCE_COUNTER() PerformanceCounter __PC(__func__)
#define PERFORMANCE_COUNTER() PerformanceCounter __PC(std::source_location::current().function_name())
#endif

//!< アセットフォルダ (Asset folder)
const auto DDS_PATH = std::filesystem::path("..") / "DDS";
const auto DRC_PATH = std::filesystem::path("..") / "DRC";
const auto FBX_PATH = std::filesystem::path("..") / "FBX";
const auto GLTF_PATH = std::filesystem::path("..") / "GLTF";

const auto DRC_SAMPLE_PATH = std::filesystem::path("..") / "draco" / "testdata";
const auto GLTF_SAMPLE_PATH = std::filesystem::path("..") / "glTF-Sample-Models" / "2.0";

template<typename T> const T GetMin(const T& lhs, const T& rhs);
template<typename T> const T GetMax(const T& lhs, const T& rhs);

template<typename T> void AdjustScale(std::vector<T>& Vertices, const float Scale);

class Win
{
public:
	Win();
	virtual ~Win();

	//virtual void OnInitialize([[maybe_unused]] HWND hWnd, [[maybe_unused]] HINSTANCE hInstance) {}
	virtual void OnCreate(HWND hWnd, [[maybe_unused]] HINSTANCE hInstance, LPCWSTR Title) { SetTitleWString(Title); GetClientRect(hWnd, &Rect); }
	//!< WM_SIZE はドラッグ中に繰り返しコールされてしまうので OnExitSizeMove() を使う
	virtual void OnSize([[maybe_unused]] HWND hWnd, [[maybe_unused]] HINSTANCE hInstance) {} 
	//!< ウインドウサイズ確定時にコールされる WM_EXITSIZEMOVE を使用する
	virtual void OnExitSizeMove(HWND hWnd, [[maybe_unused]] HINSTANCE hInstance) { GetClientRect(hWnd, &Rect); }
	virtual void OnTimer(HWND hWnd, [[maybe_unused]] HINSTANCE hInstance) { SendMessage(hWnd, WM_PAINT, 0, 0); }
	virtual void OnPaint([[maybe_unused]] HWND hWnd, [[maybe_unused]] HINSTANCE hInstance) {}
	virtual void OnKeyDown(HWND hWnd, [[maybe_unused]] HINSTANCE hInstance, const WPARAM Param) {
		switch (Param) {
		case VK_ESCAPE:
			SendMessage(hWnd, WM_DESTROY, 0, 0);
			break;
		case VK_PAUSE:
			TogglePause();
			break;
		case VK_RETURN:
		case VK_SPACE:
			Step();
			break;
		default:
			break;
		}
	}
	virtual void OnPreDestroy([[maybe_unused]] HWND hWnd, [[maybe_unused]] HINSTANCE hInstance) {}
	virtual void OnDestroy([[maybe_unused]] HWND hWnd, [[maybe_unused]] HINSTANCE hInstance) {}

#pragma region BORDERLESS
	static bool IsBorderless(HWND hWnd) { return ::GetWindowLongPtrW(hWnd, GWL_STYLE) & WS_POPUP; }
	static void ToggleBorderless(HWND hWnd);
	static bool AdjustBorderlessRect(HWND hWnd, RECT& Rect);
	static LRESULT GetBorderlessHit(HWND hWnd, const POINT& Cursor, const bool IsDrag = false, const bool IsResize = false);
#pragma endregion

	[[nodiscard]] std::filesystem::path GetFilePath(const std::string& Suffix) const { return std::filesystem::path(".") / (GetTitleString() + Suffix); }

	[[nodiscard]] static LONG GetWidth(const RECT& R) { return R.right - R.left; }
	[[nodiscard]] static LONG GetHeight(const RECT& R) { return R.bottom - R.top; }
	[[nodiscard]] static FLOAT GetAspectRatio(const FLOAT Width, const FLOAT Height) { return Width / Height; }
	[[nodiscard]] const RECT& GetRect() const { return Rect; }
	[[nodiscard]] LONG GetClientRectWidth() const { return GetWidth(GetRect()); }
	[[nodiscard]] LONG GetClientRectHeight() const { return GetHeight(GetRect()); }
	[[nodiscard]] FLOAT GetAspectRatioOfClientRect() const { return GetAspectRatio(static_cast<const FLOAT>(GetClientRectWidth()), static_cast<const FLOAT>(GetClientRectHeight())); }

public:
	static bool IsDDS(const std::filesystem::path& Path) {
		std::ifstream In(data(Path.string()), std::ios::in | std::ios::binary);
		if (!In.fail()) {
			std::array<uint32_t, 2> Header = { 0, 0 };
			In.read(reinterpret_cast<char*>(data(Header)), sizeof(Header));
			In.close();
			Logf("IsDDS == %s\n", 0x20534444 == Header[0] && 124 == Header[1] ? "true" : "false");
			return 0x20534444 == Header[0] && 124 == Header[1];
		}
		return false;
	}
	static bool IsKTX(const std::filesystem::path& Path) {
		std::ifstream In(data(Path.string()), std::ios::in | std::ios::binary);
		if (!In.fail()) {
			std::array<uint32_t, 4> Header = { 0, 0, 0, 0 };
			In.read(reinterpret_cast<char*>(data(Header)), sizeof(Header));
			In.close();
			Logf("IsKTX == %s\n", 0x58544bab == Header[0] && 0xbb313120 == Header[1] && 0x0a1a0a0d == Header[2] && 0x04030201 == Header[3] ? "true" : "false");
			return 0x58544bab == Header[0] && 0xbb313120 == Header[1] && 0x0a1a0a0d == Header[2] && 0x04030201 == Header[3];
		}
		return false;
	}

	[[nodiscard]] static std::string ToString(std::wstring_view WStr) {
#if 1
		std::vector<CHAR> Buffer(WideCharToMultiByte(CP_UTF8, 0, data(WStr), -1, nullptr, 0, nullptr, nullptr));
		WideCharToMultiByte(CP_UTF8, 0, data(WStr), -1, data(Buffer), static_cast<int>(size(Buffer)), nullptr, nullptr);
		return std::string(cbegin(Buffer), --cend(Buffer));
#elif 0
		//!< 時代遅れ(obsolescent)
		std::vector<CHAR> Buffer(size(WStr) + 1);
		size_t i; 
		wcstombs_s(&i, data(Buffer), size(Buffer), data(WStr), _TRUNCATE);
		return std::string(cbegin(Buffer), --cend(Buffer));
#elif 0
		//!< 非推奨(deprecated)
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> Converter;
		return Converter.to_bytes(WStr);
#else
		//!< NG :  C2220 //C4244
		return std::string(cbegin(WStr), cend(WStr));
#endif
	}
	[[nodiscard]] static std::wstring ToWString(std::string_view Str) {
#if 0
		std::vector<wchar_t> Buffer(MultiByteToWideChar(CP_UTF8, 0, data(Str), -1, nullptr, 0));
		MultiByteToWideChar(CP_UTF8, 0, data(Str), -1, data(Buffer), static_cast<int>(size(Buffer)));
		return std::wstring(cbegin(Buffer), cend(Buffer));
#elif 0
		//!< 時代遅れ(obsolescent)
		std::vector<wchar_t> Buffer(size(Str) + 1);
		size_t i; 
		mbstowcs_s(&i, data(Buffer), size(Buffer), data(Str), _TRUNCATE);
		return std::wstring(cbegin(Buffer), cend(Buffer));
#elif 0
		//!< 非推奨(deprecated)
		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> Converter;
		return Converter.from_bytes(Str);
#else
		return std::wstring(std::cbegin(Str), std::cend(Str));
#endif
	}

	[[nodiscard]] virtual std::string GetTitleString() const { return TitleString; }
	void SetTitleWString(LPCWSTR Title) { TitleString = ToString(Title); }

	//[[nodiscard]] bool FindDirectory(std::string_view Target, std::filesystem::path& Path) {
	//	auto Cur = std::filesystem::current_path();
	//	while (Cur.has_parent_path()) {
	//		for (auto i : std::filesystem::recursive_directory_iterator(Cur)) {
	//			if (i.is_directory() && std::equal(crbegin(Target), crend(Target), crbegin(i.path().string()))) {
	//				Path = i.path();
	//				return true;
	//			}
	//		}
	//		Cur = Cur.parent_path();
	//	}
	//	return false;
	//}
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
	template <typename ... T> static void Logf(const char* Format, T const & ... Args) { _Logf(LogType::Log, Format, Args ...); }
	template <typename ... T> static void Logf(const WCHAR* Format, T const & ... Args) { _Logf(LogType::Log, Format, Args ...); }
	template <typename ... T> static void Warningf(const char* Format, T const & ... Args) { _Logf(LogType::Warning, Format, Args ...); }
	template <typename ... T> static void Warningf(const WCHAR* Format, T const & ... Args) { _Logf(LogType::Warning, Format, Args ...); }
	template <typename ... T> static void Errorf(const char* Format, T const & ... Args) { _Logf(LogType::Error, Format, Args ...); }
	template <typename ... T> static void Errorf(const WCHAR* Format, T const & ... Args) { _Logf(LogType::Error, Format, Args ...); }
	template <typename T> static void LogOK(T Str);
	template <typename T> static void LogNG(T Str);

public:
	bool IsPause() const { return ControlFlag.test(0); }
	void Pause() { ControlFlag.set(0); }
	void UnPause() { ControlFlag.reset(0); }
	void TogglePause() { IsPause() ? UnPause() : Pause(); }

	bool IsStep() const { return ControlFlag.test(1); }
	void Step() { ControlFlag.set(1); }
	void UnStep() { ControlFlag.reset(1); }
	bool ProcessStep() {
		if (IsStep()) {
			UnStep();
			return true;
		}
		return false;
	}
	bool IsUpdate() { return !IsPause() || ProcessStep(); }

	void Pushd(const std::filesystem::path& Path) {
		PathStack.push(Path);
		std::filesystem::current_path(PathStack.top());
	}
	void Pushd() { Pushd(std::filesystem::current_path()); }
	void Popd() {
		PathStack.pop();
		if (!empty(PathStack)) {
			std::filesystem::current_path(PathStack.top());
		}
	}

protected:
	std::string TitleString;
	static constexpr UINT DeltaMsec = 1000 / 60;
	static constexpr FLOAT DeltaSec = 1.0f / 60.0f;

private:
	RECT Rect;
#ifdef DEBUG_STDOUT
	FILE* StdOut = nullptr; 
	FILE* StdErr = nullptr;
#endif
	std::bitset<32> ControlFlag;

	std::stack<std::filesystem::path> PathStack;
};

class PerformanceCounter
{
public:
	PerformanceCounter(std::string_view Label = "");
	~PerformanceCounter();
private:
	LARGE_INTEGER Start;
	std::string Label;
};

template<typename T> static [[nodiscard]] size_t IndexOf(const std::vector<T>& Vector, const T& rhs) { return static_cast<size_t>(&rhs - &*std::begin(Vector)); }

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

#ifdef USE_EXPERIMENTAL
namespace Util {
	template <typename T> concept HasSize = requires (T lhs) { lhs.size(); };
	template <HasSize T> size_t size(const T& lhs) { return lhs.size(); }

	template <typename T> concept HasData = requires (T lhs) { lhs.data(); };
	template <HasData T> const void* data(const T& lhs) { return lhs.data(); }
}
#endif