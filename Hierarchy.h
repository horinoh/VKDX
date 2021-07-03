#pragma once

class Hierarchy {
public:
	static [[nodiscard]] std::string GetEnv(std::string_view Name) {
#if 1
		//!< std::getenv() ‚ðŽg‚¤‚Æ“{‚ç‚ê‚é‚Ì‚Å _dupenv_s() ‚ðŽg‚¤
		std::string Value;
		char* Buf;
		size_t Len;
		if (0 == _dupenv_s(&Buf, &Len, data(Name))) {
			Value = Buf;
			free(Buf);
		}
		return Value;
#else
#pragma warning(push)
#pragma warning(disable : 4996)
		return std::getenv(data(Name));
#pragma warning(pop)
#endif
	}

protected:
#ifdef _DEBUG
	void Tabs() { for (auto i = 0; i < TabDepth; ++i) { std::cout << "\t"; } }
	void PushTab() { ++TabDepth; }
	void PopTab() { --TabDepth; }
#else
	void Tabs() {}
	void PushTab() {}
	void PopTab() {}
#endif
#ifdef _DEBUG
	uint8_t TabDepth = 0;
#endif
};