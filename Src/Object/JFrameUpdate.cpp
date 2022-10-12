#include"JFrameUpdate.h"

namespace JinEngine
{
	int JFrameBuff::GetFrameBuffOffset()const noexcept
	{
		return frameBuffOffset;
	}
	void JFrameBuff::SetFrameBuffOffset(int value)noexcept
	{
		frameBuffOffset = value;
	}

	int JFrameBuffUserInterface::CallGetFrameBuffOffset(IFrameBuff& iFrameBuff)const noexcept
	{
		return iFrameBuff.GetFrameBuffOffset();
	}
	void JFrameBuffManagerInterface::CallSetFrameBuffOffset(IFrameBuff& iFrameBuff, int value)const noexcept
	{
		iFrameBuff.SetFrameBuffOffset(value);
	}

	void JFrameDirtyTrigger::SetFrameDirty()noexcept {}


	bool JFrameDirty::IsFrameDirted()const noexcept
	{
		return frameDirty > 0;
	}
	void JFrameDirty::SetFrameDirty()noexcept
	{
		frameDirty = Graphic::gNumFrameResources;
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
	int JFrameDirty::GetFrameDirty()noexcept
	{
		return frameDirty;
	}

}