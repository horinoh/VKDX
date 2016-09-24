#pragma once

//!< fopen �łȂ� fopen_s ���g���Ɠ{���邪�Agli �̃R�[�h�͏������������Ȃ��̂� warning ��}������
#pragma warning (push)
#pragma warning (disable : 4996)
#include <gli/gli.hpp>
#pragma warning (pop)

#include "VKExt.h"

class VKImage : public VKExt
{
private:
	using Super = VKExt;
protected:
	void LoadDDS(const std::string& Path);
	virtual void LoadTexture(const std::string& Path) override { LoadDDS(Path); }
};

