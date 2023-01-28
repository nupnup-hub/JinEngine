#include"JFrameUpdate.h"

namespace JinEngine
{
	int JFrameBuff1::GetFrameBuffOffset()const noexcept
	{
		return frameBuffOffset;
	}
	void JFrameBuff1::SetFrameBuffOffset(int value)noexcept
	{
		frameBuffOffset = value;
	}
	int JFrameBuff2::GetFirstFrameBuffOffset()const noexcept
	{
		return frameBuffOffset00;
	}
	int JFrameBuff2::GetSecondFrameBuffOffset()const noexcept
	{
		return frameBuffOffset01;
	}
	void JFrameBuff2::SetFirstFrameBuffOffset(int value)noexcept
	{
		frameBuffOffset00 = value;
	}
	void JFrameBuff2::SetSecondFrameBuffOffset(int value)noexcept
	{
		frameBuffOffset01 = value;
	}

	int JFrameBuffUserAccess::CallGetFrameBuffOffset(IFrameBuff1& iFrameBuff)const noexcept
	{
		return iFrameBuff.GetFrameBuffOffset();
	}
	int JFrameBuffUserAccess::CallGetFirstFrameBuffOffset(IFrameBuff2& iFrameBuff)const noexcept
	{
		return iFrameBuff.GetFirstFrameBuffOffset();
	}
	int JFrameBuffUserAccess::CallGetSecondFrameBuffOffset(IFrameBuff2& iFrameBuff)const noexcept
	{
		return iFrameBuff.GetSecondFrameBuffOffset();
	}
	void JFrameBuffManagerAccess::CallSetFrameBuffOffset(IFrameBuff1& iFrameBuff, int value)const noexcept
	{
		iFrameBuff.SetFrameBuffOffset(value);
	}
	void JFrameBuffManagerAccess::CallSetFirstFrameBuffOffset(IFrameBuff2& iFrameBuff, int value)const noexcept
	{
		iFrameBuff.SetFirstFrameBuffOffset(value);
	}
	void JFrameBuffManagerAccess::CallSetSecondFrameBuffOffset(IFrameBuff2& iFrameBuff, int value)const noexcept
	{
		iFrameBuff.SetSecondFrameBuffOffset(value);
	}
	void JFrameDirtyTrigger::SetFrameDirty()noexcept {}


	bool JFrameDirty::IsFrameDirted()const noexcept
	{
		return frameDirty > 0;
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
	int JFrameDirty::GetFrameDirty()noexcept
	{
		return frameDirty;
	}

}