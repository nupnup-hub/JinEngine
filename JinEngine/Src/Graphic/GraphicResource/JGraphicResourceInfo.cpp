#include"JGraphicResourceInfo.h"
#include"../JGraphicDrawList.h" 
#include"../JGraphicPrivate.h"
#include<d3d12.h>

namespace JinEngine
{
	namespace Graphic
	{
		J_GRAPHIC_RESOURCE_TYPE JGraphicResourceInfo::GetGraphicResourceType()const noexcept
		{ 
			return graphicResourceType;
		}
		uint JGraphicResourceInfo::GetWidth()const noexcept
		{ 
			return resource->GetDesc().Width;
		}
		uint JGraphicResourceInfo::GetHeight()const noexcept
		{
			return resource->GetDesc().Height;
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
		void JGraphicResourceInfo::SetArrayIndex(const int newValue)
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
		bool JGraphicResourceInfo::Destroy()
		{ 
			return JGraphicPrivate::ResourceInterface::DestroyGraphicTextureResource(this); 
		}
		JGraphicResourceInfo::JGraphicResourceInfo(const J_GRAPHIC_RESOURCE_TYPE graphicResourceType, Microsoft::WRL::ComPtr<ID3D12Resource>&& resource)
			:graphicResourceType(graphicResourceType), resource(std::move(resource))
		{}
		JGraphicResourceInfo::~JGraphicResourceInfo()
		{
			resource->Release();
			resource.Reset();
		}
	}
}