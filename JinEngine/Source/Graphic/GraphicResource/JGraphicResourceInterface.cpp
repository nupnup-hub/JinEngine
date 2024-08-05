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


#include"JGraphicResourceInterface.h"
#include"../JGraphic.h"
#include"../JGraphicPrivate.h"
#include"../JGraphicDrawList.h" 
#include"../../Core/Math/JVectorExtend.h"
//Debug
//#include"../../Develop/Debug/JDevelopDebug.h"

#define COMMON_RESOURCE_PROCESS(type, funcName, ...)				\
if (!HasSpace(J_GRAPHIC_RESOURCE_TYPE::type))						\
	return false;													\
auto gUser = JGraphicPrivate::ResourceInterface::funcName(__VA_ARGS__);	\
if (gUser == nullptr)												\
	return false;													\
AddInfo(gUser);														\
return true;														\


namespace JinEngine
{
	namespace Graphic
	{
		bool JGraphicResourceInterface::CreateResource(const JGraphicResourceCreationDesc& createDesc, const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			int nextIndex = NextResourceIndex(rType);
			if (nextIndex != invalidIndex)
				return false;

			auto gUser = JGraphicPrivate::ResourceInterface::CreateResource(createDesc, rType);
			if (gUser == nullptr)
				return false;

			AddInfo(gUser, nextIndex);
			return true;
		}
		bool JGraphicResourceInterface::CreateOption(JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE option)
		{
			if (!info.IsValid())
				return false;

			return JGraphicPrivate::ResourceInterface::CreateOption(info, option);
		}
		bool JGraphicResourceInterface::DestroyGraphicResource(JUserPtr<JGraphicResourceInfo>& info)
		{
			if (!info.IsValid())
				return false;

			return JGraphicPrivate::ResourceInterface::DestroyGraphicTextureResource(info.Release());
		}
		bool JGraphicResourceInterface::DestroyGraphicOption(JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE option)
		{
			if (!info.IsValid())
				return false;

			return JGraphicPrivate::ResourceInterface::DestroyGraphicOption(info, option);
		}
		bool JGraphicResourceInterface::SetMipmap(const JUserPtr<JGraphicResourceInfo>& info, const JTextureCreationDesc& createDesc)
		{
			if (!info.IsValid())
				return false;

			return JGraphicPrivate::ResourceInterface::SetMipmap(info, createDesc);
		}
		bool JGraphicResourceInterface::SetTextureDetail(const JUserPtr<JGraphicResourceInfo>& info, const JConvertColorDesc& convertDesc)
		{
			if (!info.IsValid())
				return false;

			return JGraphicPrivate::ResourceInterface::SetTextureDetail(info, convertDesc);
		} 
		uint JGraphicResourceInterface::GetResourceWidth(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept
		{
			auto info = GetGraphicInfo(rType, dataIndex);
			return info != nullptr ? info->GetWidth() : 0;
		}
		uint JGraphicResourceInterface::GetResourceHeight(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept
		{
			auto info = GetGraphicInfo(rType, dataIndex);
			return info != nullptr ? info->GetHeight() : 0;
		}
		JVector2F JGraphicResourceInterface::GetResourceSize(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept
		{
			auto info = GetGraphicInfo(rType, dataIndex);
			return info != nullptr ? info->GetResourceSize() : JVector2<uint>::Zero();
		}
		JVector2F JGraphicResourceInterface::GetResourceInvSize(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept
		{
			auto info = GetGraphicInfo(rType, dataIndex);
			return info != nullptr ? (1.0f / JVector2F(info->GetResourceSize())) : JVector2F::Zero();
		}
		int JGraphicResourceInterface::GetResourceArrayIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept
		{
			auto info = GetGraphicInfo(rType, dataIndex);
			return info != nullptr ? info->GetArrayIndex() : invalidIndex;
		}
		int JGraphicResourceInterface::GetResourceArrayIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
		{
			auto info = GetGraphicInfo(rType, GetResourceIndex(rType, taskType));
			return info != nullptr ? info->GetArrayIndex() : invalidIndex;
		}
		int JGraphicResourceInterface::GetHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept
		{
			auto info = GetGraphicInfo(rType, dataIndex);
			return info != nullptr ? info->GetHeapIndexStart(bType) : invalidIndex;
		}
		int JGraphicResourceInterface::GetOptionHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint dataIndex)const noexcept
		{
			auto info = GetGraphicInfo(rType, dataIndex);
			return info != nullptr ? info->GetOptionHeapIndexStart(bType, opType) : invalidIndex;
		}
		uint JGraphicResourceInterface::GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept
		{
			auto info = GetGraphicInfo(rType, dataIndex);
			return info != nullptr ? info->GetViewCount(bType) : 0;
		}
		uint JGraphicResourceInterface::GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
		{
			int dataIndex = GetResourceIndex(rType, taskType);
			return dataIndex != invalidIndex ? GetViewCount(rType, bType, dataIndex) : 0;
		}
		ResourceHandle JGraphicResourceInterface::GetGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint bIndex, const uint dataIndex) const noexcept
		{
			auto info = GetGraphicInfo(rType, dataIndex);
			if (info != nullptr)
				return info->GetResourceGpuHandle(bType, bIndex);
			else
				return nullptr;
		}
		ResourceHandle JGraphicResourceInterface::GetOptionGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint bIndex, const uint dataIndex) const noexcept
		{
			auto info = GetGraphicInfo(rType, dataIndex);
			if (info != nullptr)
				return info->GetResourceOptionGpuHandle(bType, opType, bIndex);
			else
				return nullptr;
		}
		int JGraphicResourceInterface::GetFirstResourceArrayIndex()const noexcept
		{
			auto info = GetFirstGraphicInfo();
			return info != nullptr ? info->GetArrayIndex() : -1;
		}
		int JGraphicResourceInterface::GetFirstResourceHeapStart(const J_GRAPHIC_BIND_TYPE bType)const noexcept
		{
			auto info = GetFirstGraphicInfo();
			return info != nullptr ? info->GetHeapIndexStart(bType) : -1;
		}
		JVector2F JGraphicResourceInterface::GetFirstResourceSize()const noexcept
		{
			auto info = GetFirstGraphicInfo();
			return info != nullptr ? info->GetResourceSize() : JVector2<uint>::Zero();
		}
		JVector2F JGraphicResourceInterface::GetFirstResourceInvSize()const noexcept
		{
			auto info = GetFirstGraphicInfo();
			return info != nullptr ? (1.0f / JVector2F(info->GetResourceSize())) : JVector2F::Zero();
		}
		J_GRAPHIC_RESOURCE_TYPE JGraphicResourceInterface::GetFirstResourceType()const noexcept
		{
			auto info = GetFirstGraphicInfo();
			return info != nullptr ? info->GetGraphicResourceType() : (J_GRAPHIC_RESOURCE_TYPE)invalidIndex;
		}
		ResourceHandle JGraphicResourceInterface::GetFirstGpuHandle(const J_GRAPHIC_BIND_TYPE bType) const noexcept
		{
			auto info = GetFirstGraphicInfo();
			if (info != nullptr)
				return info->GetResourceGpuHandle(bType, 0);
			else
				return nullptr;
		}
		J_GRAPHIC_MIP_MAP_TYPE JGraphicResourceInterface::GetFirstMipmapType()const noexcept
		{
			auto info = GetFirstGraphicInfo();
			return info != nullptr ? info->GetMipmapType() : J_GRAPHIC_MIP_MAP_TYPE::NONE;
		}
		bool JGraphicResourceInterface::IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept
		{
			return GetGraphicInfo(rType, dataIndex) != nullptr;
		}
		bool JGraphicResourceInterface::IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
		{
			return GetGraphicInfo(rType, GetResourceIndex(rType, taskType)) != nullptr;
		} 
		bool JGraphicResourceInterface::HasFirstHandle()const noexcept
		{
			return GetFirstGraphicInfo() != nullptr;
		}
		bool JGraphicResourceInterface::HasOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
		{
			auto info = GetGraphicInfo(rType, GetResourceIndex(rType, taskType));
			return info != nullptr ? info->HasOption(opType) : false;
		}
		bool JGraphicResourceInterface::HasFirstOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)const noexcept
		{
			auto info = GetFirstGraphicInfo();
			return info != nullptr ? info->HasOption(opType) : false;
		}
		bool JGraphicResourceInterface::HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
		{
			return NextResourceIndex(rType, taskType) != invalidIndex;
		} 
		int JGraphicResourceInterface::NextResourceIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
		{
			const uint allocableCount = GetAllocableResourceCount(rType, taskType);
			const uint offSet = GetResourceIndex(rType, taskType);
			if (allocableCount == 0)
				return invalidIndex;

			int nextIndex = invalidIndex;
			for (uint i = 0; i < allocableCount; ++i)
			{
				if (GetGraphicInfo(rType, i + offSet) == nullptr)
				{
					nextIndex = i;
					break;
				}
			}
			return nextIndex; 
		} 
		bool JGraphicResourceInterface::TryFirstResourceMipmapBind(_Out_ std::vector<ResourceHandle>& gpuHandle, _Out_ std::vector<Core::JDataHandle>& dataHandle)const
		{
#ifdef DEVELOP 
			auto info = GetFirstGraphicInfo();
			return info != nullptr ? JGraphicPrivate::ResourceInterface::MipmapBindForDebug(info, gpuHandle, dataHandle) : false;
#else
			return false;
#endif
		}
		void JGraphicResourceInterface::ClearFirstResourceMipmapBind(_Inout_ std::vector<Core::JDataHandle>& dataHandle)
		{
			JGraphicPrivate::ResourceInterface::ClearMipmapBind(dataHandle);
		}

		bool JGraphicSingleResourceHolder::DestroyGraphicResource()
		{
			return JGraphicResourceInterface::DestroyGraphicResource(info);
		}
		void JGraphicSingleResourceHolder::AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint index)
		{
			info = newInfo;
		}
		uint JGraphicSingleResourceHolder::GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return info != nullptr ? 1 : 0;
		}   
		std::vector<J_GRAPHIC_RESOURCE_TYPE> JGraphicSingleResourceHolder::GetResourceTypeVec()const noexcept
		{
			return info != nullptr ? std::vector<J_GRAPHIC_RESOURCE_TYPE>{info->GetGraphicResourceType()} : std::vector<J_GRAPHIC_RESOURCE_TYPE>{};
		}
		JUserPtr<JGraphicResourceInfo> JGraphicSingleResourceHolder::GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept
		{
			return info != nullptr && info->GetGraphicResourceType() == rType && dataIndex == 0 ? info : nullptr;
		}
		JUserPtr<JGraphicResourceInfo> JGraphicSingleResourceHolder::GetFirstGraphicInfo()const noexcept
		{
			return info;
		}
		bool JGraphicSingleResourceHolder::HasGraphicResourceHandle()const noexcept
		{
			return info != nullptr;
		} 
		bool JGraphicSingleResourceHolder::HasFixedSpaceByTaskType()const noexcept
		{
			return false;
		} 

		void JGraphicTypePerSingleResourceHolder::DestroyGraphicResource(const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			JGraphicResourceInterface::DestroyGraphicResource(info[(uint)rType]);
		}
		void JGraphicTypePerSingleResourceHolder::DestroyAllTexture()
		{
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				JGraphicResourceInterface::DestroyGraphicResource(info[i]);
		}
		void JGraphicTypePerSingleResourceHolder::AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex)
		{
			const uint index = (uint)newInfo->GetGraphicResourceType();
			info[index] = newInfo;
		}
		uint JGraphicTypePerSingleResourceHolder::GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return info[(uint)rType] != nullptr ? 1 : 0;
		} 
		uint JGraphicTypePerSingleResourceHolder::GetAllocableResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
		{
			return GetResourceIndex(rType, taskType) != invalidIndex;
		}
		std::vector<J_GRAPHIC_RESOURCE_TYPE> JGraphicTypePerSingleResourceHolder::GetResourceTypeVec()const noexcept
		{
			std::vector<J_GRAPHIC_RESOURCE_TYPE> res;
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			{
				if (info[i] != nullptr)
					res.push_back((J_GRAPHIC_RESOURCE_TYPE)i);
			}
			return res;

		}
		JUserPtr<JGraphicResourceInfo> JGraphicTypePerSingleResourceHolder::GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept
		{
			return info[(uint)rType] != nullptr && dataIndex == 0 ? info[(uint)rType] : nullptr;
		}
		JUserPtr<JGraphicResourceInfo> JGraphicTypePerSingleResourceHolder::GetFirstGraphicInfo()const noexcept
		{
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			{
				if (info[i] != nullptr)
					return info[i];
			}
			return nullptr;
		} 
		bool JGraphicTypePerSingleResourceHolder::HasFixedSpaceByTaskType()const noexcept
		{
			return false;
		} 

		std::vector<JUserPtr<JGraphicResourceInfo>>& JGraphicMultiResourceHolder::MultiResourceInfo::operator[](const J_GRAPHIC_RESOURCE_TYPE type)noexcept
		{
			return vec[(uint)type];
		}
		const std::vector<JUserPtr<JGraphicResourceInfo>>& JGraphicMultiResourceHolder::MultiResourceInfo::operator[](const J_GRAPHIC_RESOURCE_TYPE type)const noexcept
		{
			return vec[(uint)type];
		}

		void JGraphicMultiResourceHolder::DestroyGraphicResource(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)
		{
			if (!IsValidHandle(rType, dataIndex))
				return;

			JGraphicResourceInfo::Destroy(info[rType][dataIndex].Release());
			info[rType].erase(info[rType].begin() + dataIndex);
		}
		void JGraphicMultiResourceHolder::DestroyGraphicResource(const J_GRAPHIC_RESOURCE_TYPE rType)
		{
			const uint count = (uint)info[rType].size();
			for (uint i = 0; i < count; ++i)
				JGraphicResourceInfo::Destroy(info[rType][i].Release());
			info[rType].clear();
		}
		void JGraphicMultiResourceHolder::DestroyAllTexture()
		{
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
				DestroyGraphicResource((J_GRAPHIC_RESOURCE_TYPE)i);
		}
		void JGraphicMultiResourceHolder::AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex)
		{
			info[newInfo->GetGraphicResourceType()].push_back(newInfo);
		}
		uint JGraphicMultiResourceHolder::GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
		{
			return (uint)info[rType].size();
		} 
		uint JGraphicMultiResourceHolder::GetAllocableResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept
		{
			return GetResourceIndex(rType, taskType) != invalidIndex ? UINT_MAX : 0;
		}
		std::vector<J_GRAPHIC_RESOURCE_TYPE> JGraphicMultiResourceHolder::GetResourceTypeVec()const noexcept
		{
			std::vector<J_GRAPHIC_RESOURCE_TYPE> res;
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			{
				auto& vec = info.vec[i];
				for (const auto& data : vec)
				{
					if (data != nullptr)
						res.push_back((J_GRAPHIC_RESOURCE_TYPE)i);
				}
			}
			return res;
		}
		JUserPtr<JGraphicResourceInfo> JGraphicMultiResourceHolder::GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept
		{
			return info[rType].size() > dataIndex ? info[rType][dataIndex] : nullptr;
		}
		JUserPtr<JGraphicResourceInfo> JGraphicMultiResourceHolder::GetFirstGraphicInfo()const noexcept
		{
			for (uint i = 0; i < (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT; ++i)
			{
				auto& vec = info[(J_GRAPHIC_RESOURCE_TYPE)i];
				if (vec.size() > 0)
				{
					for (const auto& data : vec)
					{
						if (data != nullptr)
							return data;
					}
				}
			}
			return nullptr;
		} 
		bool JGraphicMultiResourceHolder::HasFixedSpaceByTaskType() const noexcept
		{
			return false;
		}
 
	}
}