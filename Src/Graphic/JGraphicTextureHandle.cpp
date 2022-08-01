#include"JGraphicTextureHandle.h"

namespace JinEngine
{
	namespace Graphic
	{
		JGraphicTextureHandle::JGraphicTextureHandle(const J_GRAPHIC_TEXTURE_TYPE graphicResourceType, uint width, uint height)
			:graphicResourceType(graphicResourceType),
			width(width), height(height)
		{}
		JGraphicTextureHandle::~JGraphicTextureHandle() {}

		J_GRAPHIC_TEXTURE_TYPE JGraphicTextureHandle::GetGraphicResourceType()const noexcept
		{
			return graphicResourceType;
		}
		uint JGraphicTextureHandle::GetWidth()const noexcept
		{
			return width;
		}
		uint JGraphicTextureHandle::GetHeight()const noexcept
		{
			return height;
		}
		int JGraphicTextureHandle::GetRtvHeapIndex()const noexcept
		{
			return rtvHeapIndex;
		}
		int JGraphicTextureHandle::GetDsvHeapIndex()const noexcept
		{
			return dsvHeapIndex;
		}
		int JGraphicTextureHandle::GetSrvHeapIndex()const noexcept
		{
			return srvHeapIndex;
		}
		int JGraphicTextureHandle::GetResourceVectorIndex()const noexcept
		{
			return resourceVectorIndex;
		}
	}
}