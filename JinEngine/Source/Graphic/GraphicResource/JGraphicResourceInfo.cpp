/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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
		int JGraphicResourceInfo::GetOptionHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)const noexcept
		{
			return optionalInfo != nullptr ? optionalInfo->viewInfo[(uint)opType][(uint)bindType].stIndex : invalidIndex;
		}
		int JGraphicResourceInfo::GetMinValidHeapIndexStart(const J_GRAPHIC_BIND_TYPE bindType)const noexcept
		{
			if (optionalInfo != nullptr)
			{
				int infoViewSt = GetHeapIndexStart(bindType);
				int minValid = INT_MAX;
				for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_OPTION_TYPE::COUNT; ++i)
				{
					int opViewSt = optionalInfo->viewInfo[i][(uint)bindType].stIndex;
					if (opViewSt != invalidIndex && opViewSt < minValid)
						minValid = opViewSt;
				} 
				minValid = infoViewSt != invalidIndex ? min(infoViewSt, minValid) : minValid;
				return minValid != INT_MAX ? minValid : invalidIndex;
			}
			else
				return GetHeapIndexStart(bindType);
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