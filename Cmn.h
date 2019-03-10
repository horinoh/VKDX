#pragma once

class Cmn
{
public:
	static size_t RoundUp(const size_t Size, const uint16_t Aligh) { return (Size + Aligh) & ~Aligh; }

};
