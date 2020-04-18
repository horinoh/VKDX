#pragma once

class Cmn
{
public:
	static size_t RoundUp(const size_t Size, const uint16_t Aligh) { return (Size + Aligh) & ~Aligh; }
	static size_t RoundUp256(const size_t Size) { return RoundUp(Size, 0xff); }
};
