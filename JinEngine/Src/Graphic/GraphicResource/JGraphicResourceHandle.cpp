#include"JGraphicResourceHandle.h"
#include"../JGraphicDrawList.h"
#include"../JGraphic.h"

namespace JinEngine
{
	namespace Graphic
	{
		J_GRAPHIC_RESOURCE_TYPE JGraphicResourceHandle::GetGraphicResourceType()const noexcept
		{ 
			return graphicResourceType;
		}
		uint JGraphicResourceHandle::GetWidth()const noexcept
		{
			return width;
		}
		uint JGraphicResourceHandle::GetHeight()const noexcept
		{
			return height;
		}
		int JGraphicResourceHandle::GetArrayIndex()const noexcept
		{
			return resourceArrayIndex;
		}
		int JGraphicResourceHandle::GetHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType)const noexcept
		{
			return viewInfo[(int)bindType].stIndex;
		}
		uint JGraphicResourceHandle::GetViewCount(const J_GRAPHIC_BIND_TYPE bindType)const noexcept
		{ 
			return viewInfo[(int)bindType].count;
		}
		void JGraphicResourceHandle::SetArrayIndex(const int newValue)
		{
			resourceArrayIndex = newValue;
		}
		void JGraphicResourceHandle::SetHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType, const int newValue)
		{
			viewInfo[(int)bindType].stIndex = newValue;
		}
		void JGraphicResourceHandle::SetViewCount(const J_GRAPHIC_BIND_TYPE bindType, const uint newValue)
		{
			viewInfo[(int)bindType].count = newValue;
		}
		bool JGraphicResourceHandle::Destroy(JGraphicResourceHandle** handle)
		{ 
			if (handle == nullptr || *handle == nullptr)
				return false;

			return JGraphic::Instance().ResourceInterface()->DestroyGraphicTextureResource(handle);
		}
		JGraphicResourceHandle::JGraphicResourceHandle(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, const uint width, const uint height)
			:graphicResourceType(graphicResourceType),
			width(width), height(height)
		{}
		JGraphicResourceHandle::~JGraphicResourceHandle() {}
	}
}