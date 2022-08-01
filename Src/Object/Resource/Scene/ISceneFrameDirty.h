#pragma once  

namespace JinEngine
{ 
	namespace Graphic
	{
		class JGraphicDrawList;
	}
	class ISceneFrameDirty abstract
	{
		friend class PreviewResourceScene; 
		friend class Graphic::JGraphicDrawList;
	public:
		virtual ~ISceneFrameDirty() = default;
	private:
		virtual void SetAllComponentDirty()noexcept = 0;
		virtual void SetAllTransformDirty()noexcept = 0;
		virtual void SetAllRenderItemDirty()noexcept = 0;
		virtual void SetAllAnimatorDirty()noexcept = 0;
		virtual void SetAllCameraDirty()noexcept = 0;
		virtual void SetAllLightDirty()noexcept = 0;
	};
}