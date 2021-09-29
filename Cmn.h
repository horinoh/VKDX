#pragma once

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) if(nullptr != x) { delete x; x = nullptr; }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(x) if(nullptr != x) { delete [] x; x = nullptr; }
#endif

#ifndef VERIFY
#ifdef  _DEBUG
//#define VERIFY(x) assert(x)
#define VERIFY(x) if(!(x)) { DEBUG_BREAK(); }
//#define VERIFY(x) if (!(x)) { throw std::runtime_error("VERIFY failed"); }
#else
#define VERIFY(x) (x)
#endif
#endif

#include <vector>

class Cmn
{
public:
	static constexpr size_t RoundUpMask(const size_t Size, const size_t Mask) { return (Size + Mask) & ~Mask; }
	static constexpr size_t RoundUp(const size_t Size, const size_t Align) { return RoundUpMask(Size, Align - 1); }
	static constexpr size_t RoundUp256(const size_t Size) { return RoundUpMask(Size, 0xff); }

	template<typename T> static constexpr size_t TotalSizeOf(const std::vector<T>& rhs) { return sizeof(T) * size(rhs); }
};
