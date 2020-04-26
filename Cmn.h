#pragma once

class Cmn
{
public:
	static bool IsAligned(const size_t Size, const size_t Align) { return !(Size & ~Align); }
	static size_t RoundDown(const size_t Size, const size_t Align) {
		if (IsAligned(Size, Align)) { return Size; }
		return (Size / Align) * Align;
	}
	static size_t RoundUp(const size_t Size, const size_t Align) {
		if (IsAligned(Size, Align)) { return Size; }
		return RoundDown(Size, Align) + Align;
	}
	static size_t RoundUp256(const size_t Size) { return RoundUp(Size, 0x100); }
};
