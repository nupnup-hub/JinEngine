#pragma once
#include"../Graphic/FrameResource/JFrameResourceCash.h" 

namespace JinEngine
{  
	namespace Graphic
	{
		class JGraphicImpl;
	}

	template<typename ...FrameConstants>
	class IFrameUpdate
	{
	private:
		friend Graphic::JGraphicImpl;
	protected:
		virtual ~IFrameUpdate() = default;
	private:
		bool CallUpdateFrame(FrameConstants&... frameConstant)
		{
			return UpdateFrame(frameConstant...);
		}
		virtual bool UpdateFrame(FrameConstants&... frameConstant) = 0;
	};

	template<typename ...FrameConstant>
	class JFrameInterface : public IFrameUpdate<FrameConstant...>
	{
	private:
		int frameDirty;
	protected:
		bool IsFrameDirted()const noexcept
		{
			return frameDirty > 0;
		}
		virtual void SetFrameDirty()noexcept
		{
			frameDirty = Graphic::gNumFrameResources;
		}
		void MinusFrameDirty()noexcept
		{
			--frameDirty;
			if (frameDirty < 0)
				frameDirty = 0;
		}
		void OffFrameDirty()noexcept
		{
			frameDirty = 0;
		}
	};

	template<typename Listener>
	class JShareFrameInterface
	{
	private:
		friend Listener;
	private:
		int frameDirty;
	protected:
		bool IsFrameDirted()const noexcept
		{
			return frameDirty > 0;
		}
		void SetFrameDirty()noexcept
		{
			frameDirty = Graphic::gNumFrameResources;
		}
		void MinusFrameDirty()noexcept
		{
			--frameDirty;
			if (frameDirty < 0)
				frameDirty = 0;
		}
		void OffFrameDirty()noexcept
		{
			frameDirty = 0;
		}
	};
}