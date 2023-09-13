#include"JGraphicResourceInfo.h"  
#include"../JGraphicPrivate.h"
#include"../../Core/Reflection/JTypeBase.h"

namespace JinEngine
{
	namespace Graphic
	{
		static bool IsOwnerType(const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			switch (rType)
			{
			case JinEngine::Graphic::J_GRAPHIC_RESOURCE_TYPE::SWAP_CHAN:	//owner is swapchain class
				return false;
			default:
				return true;
			}
		}

		J_GRAPHIC_RESOURCE_TYPE JGraphicResourceInfo::GetGraphicResourceType()const noexcept
		{   
			return graphicResourceType;
		}
		int JGraphicResourceInfo::GetArrayIndex()const noexcept
		{
			return resourceArrayIndex;
		}
		int JGraphicResourceInfo::GetHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType)const noexcept
		{
			return viewInfo[(int)bindType].stIndex;
		}
		uint JGraphicResourceInfo::GetViewCount(const J_GRAPHIC_BIND_TYPE bindType)const noexcept
		{
			return viewInfo[(int)bindType].count;
		}
		void JGraphicResourceInfo::SetArrayIndex(const int newValue)noexcept
		{ 
			resourceArrayIndex = newValue;
		}
		void JGraphicResourceInfo::SetHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType, const int newValue)
		{
			viewInfo[(int)bindType].stIndex = newValue;
		}
		void JGraphicResourceInfo::SetViewCount(const J_GRAPHIC_BIND_TYPE bindType, const uint newValue)
		{
			viewInfo[(int)bindType].count = newValue;
		}
		bool JGraphicResourceInfo::HasView(const J_GRAPHIC_BIND_TYPE bindType)const noexcept
		{
			return viewInfo[(int)bindType].count > 0;
		}
		bool JGraphicResourceInfo::Destroy(JGraphicResourceInfo* info)
		{   
			return JGraphicPrivate::ResourceInterface::DestroyGraphicTextureResource(info);
		} 
		JGraphicResourceInfo::JGraphicResourceInfo(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType)
			:graphicResourceType(graphicResourceType)
		{}
		JGraphicResourceInfo::~JGraphicResourceInfo()
		{}
	}
}