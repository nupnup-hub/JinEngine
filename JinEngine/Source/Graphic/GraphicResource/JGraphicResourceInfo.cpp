#include"JGraphicResourceInfo.h"  
#include"../JGraphicPrivate.h"
#include"../../Core/Reflection/JTypeBase.h"

namespace JinEngine
{
	namespace Graphic
	{
		namespace Private
		{

		}
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
		int JGraphicResourceInfo::GetOptionHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)const noexcept
		{
			return optionalInfo != nullptr ? optionalInfo->viewInfo[(uint)opType][(uint)bindType].stIndex : invalidIndex;
		}
		uint JGraphicResourceInfo::GetViewCount(const J_GRAPHIC_BIND_TYPE bindType)const noexcept
		{
			return viewInfo[(int)bindType].count;
		}
		uint JGraphicResourceInfo::GetOptionViewCount(const J_GRAPHIC_BIND_TYPE bindType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)const noexcept
		{
			return optionalInfo != nullptr ? optionalInfo->viewInfo[(uint)opType][(uint)bindType].count : 0;
		}
		J_GRAPHIC_MIP_MAP_TYPE JGraphicResourceInfo::GetMipmapType()const noexcept
		{
			return mipMapType;
		}
		void JGraphicResourceInfo::SetArrayIndex(const int newValue)noexcept
		{
			resourceArrayIndex = newValue;
		}
		void JGraphicResourceInfo::SetHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType, const int newValue)
		{
			viewInfo[(int)bindType].stIndex = newValue;
		}
		void JGraphicResourceInfo::SetHeapOptionIndexStart(const J_GRAPHIC_BIND_TYPE bindType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const int newValue)
		{
			if (optionalInfo == nullptr)
				return;

			optionalInfo->viewInfo[(uint)opType][(uint)bindType].stIndex = newValue;
		}
		void JGraphicResourceInfo::SetViewCount(const J_GRAPHIC_BIND_TYPE bindType, const uint newValue)
		{
			viewInfo[(int)bindType].count = newValue;
		}
		void JGraphicResourceInfo::SetOptionViewCount(const J_GRAPHIC_BIND_TYPE bindType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint newValue)
		{
			if (optionalInfo == nullptr)
				return;

			optionalInfo->viewInfo[(uint)opType][(uint)bindType].count = newValue;
		} 
		void JGraphicResourceInfo::SetMipmapType(const J_GRAPHIC_MIP_MAP_TYPE newMipmapType)noexcept
		{
			mipMapType = newMipmapType;
		}
		bool JGraphicResourceInfo::HasView(const J_GRAPHIC_BIND_TYPE bindType)const noexcept
		{
			return viewInfo[(int)bindType].count > 0;
		}
		bool JGraphicResourceInfo::Destroy(JGraphicResourceInfo* info)
		{
			return JGraphicPrivate::ResourceInterface::DestroyGraphicTextureResource(info);
		}
		void JGraphicResourceInfo::TryCreateOptionViewInfo()
		{
			if (optionalInfo == nullptr)
				optionalInfo = std::make_unique<OptionResourceViewInfo>();
		}
		void JGraphicResourceInfo::DestroyOptionViewInfo()
		{
			optionalInfo = nullptr;
		}
		JGraphicResourceInfo::JGraphicResourceInfo(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType)
			:graphicResourceType(graphicResourceType)
		{}
		JGraphicResourceInfo::~JGraphicResourceInfo()
		{}
	}
}