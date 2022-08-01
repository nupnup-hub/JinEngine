#pragma once
#include"../Graphic/FrameResource/JFrameResourceCash.h"
 
namespace JinEngine
{
	template<typename Base>
	class JFrameInterface : public Base
	{
	private:
		int frameDirty;
	protected:
		template<typename ...Param>
		JFrameInterface(Param&&... var)
			:Base(std::forward<Param>(var)...), frameDirty(Graphic::gNumFrameResources)
		{}
		void SetFrameDirty()
		{
			frameDirty = Graphic::gNumFrameResources;
		}
		void MinusDirty()
		{
			--frameDirty;
			if (frameDirty < 0)
				frameDirty = 0;
		}
		void ClearDirty()
		{
			frameDirty = 0;
		}
	};
}