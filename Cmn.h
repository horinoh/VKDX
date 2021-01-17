#pragma once

class Cmn
{
public:
	static size_t RoundUpMask(const size_t Size, const size_t Mask) { return (Size + Mask) & ~Mask; }
	static size_t RoundUp(const size_t Size, const size_t Align) { return RoundUpMask(Size, Align - 1); }
	static size_t RoundUp256(const size_t Size) { return RoundUpMask(Size, 0xff); }
};
