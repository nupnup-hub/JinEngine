#include"JFrameUpdate.h"

namespace JinEngine
{
	uint JFrameBuff1::GetFrameBuffOffset()const noexcept
	{
		return frameBuffOffset;
	}
	void JFrameBuff1::SetFrameBuffOffset(const uint value)noexcept
	{
		frameBuffOffset = value;
	}
	uint JFrameBuff2::GetFirstFrameBuffOffset()const noexcept
	{
		return frameBuffOffset00;
	}
	uint JFrameBuff2::GetSecondFrameBuffOffset()const noexcept
	{
		return frameBuffOffset01;
	}
	void JFrameBuff2::SetFirstFrameBuffOffset(const uint value)noexcept
	{
		frameBuffOffset00 = value;
	}
	void JFrameBuff2::SetSecondFrameBuffOffset(const uint value)noexcept
	{
		frameBuffOffset01 = value;
	}

	void JFrameDirtyTrigger::SetFrameDirty()noexcept {}


	bool JFrameDirty::IsFrameDirted()const noexcept
	{
		return frameDirty > 0;
	}
	int JFrameDirty::GetFrameDirty()noexcept
	{
		return frameDirty;
	}
	void JFrameDirty::SetFrameDirty()noexcept
	{
		frameDirty = Graphic::Constants::gNumFrameResources;
	}
	void JFrameDirty::MinusFrameDirty()noexcept
	{
		--frameDirty;
		if (frameDirty < 0)
			frameDirty = 0;
	}
	void JFrameDirty::OffFrameDirty()noexcept
	{
		frameDirty = 0;
	}
}