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


#pragma once 
#include"JGraphicResourceType.h"
#include"JGraphicResourceInfo.h" 
#include"../JGraphicOption.h"
#include"../JGraphicConstants.h" 
#include"../JGraphicEnum.h"
#include"../Image/JImageProcessingDesc.h" 
#include"../../Core/Reflection/JTypeImplBase.h"
#include"../../Core/Math/JVector.h"  

namespace JinEngine
{
	class JScene;
	class JComponent;
	class JCamera;
	namespace Core
	{
		struct JStaticMeshVertex;
		struct JSkinnedMeshVertex;
		struct J1BytePosVertex;
		class JDataHandle;
	}
	namespace Graphic
	{
		class JGraphicResourceUserInterface;
		class JCullingInterface;

		/**
		* 주로 impl class가 해당 interface을 상속한다
		* interface는 info를 소유하는 wrapper로 graphic manager와 impl간의 중간역할을 한다
		* impl이 직접 JGraphicResourceInfo를 상속하지 않은건 외부로 노출할 수 없기때문이다.
		*/

		class JGraphicResourceInterface : public Core::JTypeImplInterfacePointerHolder<JGraphicResourceInterface>
		{
		protected:
			bool CreateResource(const JGraphicResourceCreationDesc& createDesc, const J_GRAPHIC_RESOURCE_TYPE rType);
			bool CreateOption(JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE option);
		protected:
			bool DestroyGraphicResource(JUserPtr<JGraphicResourceInfo>& info);
			bool DestroyGraphicOption(JUserPtr<JGraphicResourceInfo>& info, const J_GRAPHIC_RESOURCE_OPTION_TYPE option);
		protected:
			/**
			* @brief non mipmap일 경우와 graphic api defined로 mipmap을 변경하고 싶은경우 해당하는 resource를 다시 만들 필요가있으며
			* 그밖에 경우에만 desc에 맞는 새로운 mipmap을 생성
			*/
			bool SetMipmap(const JUserPtr<JGraphicResourceInfo>& info, const JTextureCreationDesc& createDesc);
			/**
			* @brief color curve를 조정하며 현재는 reverseY만 추가된상태.
			*/
			bool SetTextureDetail(const JUserPtr<JGraphicResourceInfo>& info, const JConvertColorDesc& convertDesc);
		private:
			virtual void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo) = 0;
		public:
			uint GetResourceWidth(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			uint GetResourceHeight(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			JVector2F GetResourceSize(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			JVector2F GetResourceInvSize(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			int GetResourceArrayIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			int GetHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept;
			int GetOptionHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint dataIndex)const noexcept;
			uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept;
			Graphic::ResourceHandle GetGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint bIndex, const uint dataIndex) const noexcept;
			Graphic::ResourceHandle GetOptionGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint bIndex, const uint dataIndex) const noexcept;
			virtual uint GetDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept = 0;
		public:
			int GetFirstResourceArrayIndex()const noexcept;
			int GetFirstResourceHeapStart(const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			JVector2F GetFirstResourceSize()const noexcept;
			JVector2F GetFirstResourceInvSize()const noexcept;
			J_GRAPHIC_RESOURCE_TYPE GetFirstResourceType()const noexcept;
			Graphic::ResourceHandle GetFirstGpuHandle(const J_GRAPHIC_BIND_TYPE bType) const noexcept;
			J_GRAPHIC_MIP_MAP_TYPE GetFirstMipmapType()const noexcept;
			/**
			* @brief return resource data Index(single) or offset(multi)
			* @return invalid index if not supported type
			*/
			virtual int GetResourceDataIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept = 0;
			//for debugging
			virtual std::vector<J_GRAPHIC_RESOURCE_TYPE> GetResourceTypeVec()const noexcept = 0;
		protected:
			virtual JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept = 0;
			virtual JUserPtr<JGraphicResourceInfo> GetFirstGraphicInfo()const noexcept = 0;
		public:
			bool IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			bool HasHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			bool HasFirstHandle()const noexcept;
			bool HasOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint dataIndex);
			bool HasFirstOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType);
			virtual bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept = 0;
			virtual bool AllowHoldMultiHold(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept = 0;
		public:
			//Debug
			/**
			* @brief for debug texture mipmap
			*/
			bool TryFirstResourceMipmapBind(_Out_ std::vector<Graphic::ResourceHandle>& gpuHandle, _Out_ std::vector<Core::JDataHandle>& dataHandle)const;
			static void ClearFirstResourceMipmapBind(_Inout_ std::vector<Core::JDataHandle>& dataHandle);
		};
		class JGraphicSingleResourceHolder : public JGraphicResourceInterface
		{
		private:
			JUserPtr<JGraphicResourceInfo> info = nullptr;
		protected:
			bool DestroyGraphicResource();
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo);
		public:
			uint GetDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			int GetResourceWidth()const noexcept;
			int GetResourceHeight()const noexcept;
			std::vector<J_GRAPHIC_RESOURCE_TYPE> GetResourceTypeVec()const noexcept final;
		protected:
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final;
			JUserPtr<JGraphicResourceInfo> GetFirstGraphicInfo()const noexcept final;
		public:
			bool HasGraphicResourceHandle()const noexcept;
			bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			bool AllowHoldMultiHold(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
		};
		class JGraphicTypePerSingleResourceHolder : public JGraphicResourceInterface
		{
		private:
			JUserPtr<JGraphicResourceInfo> info[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
		protected:
			void DestroyGraphicResource(const J_GRAPHIC_RESOURCE_TYPE rType);
			void DestroyAllTexture();
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo);
		public:
			uint GetDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			std::vector<J_GRAPHIC_RESOURCE_TYPE> GetResourceTypeVec()const noexcept final;
		protected:
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final;
			JUserPtr<JGraphicResourceInfo> GetFirstGraphicInfo()const noexcept final;
		public:
			bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			bool AllowHoldMultiHold(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
		};
		class JGraphicMultiResourceHolder : public JGraphicResourceInterface
		{
		private:
			friend class JCullingInterface;
		private:
			struct MultiResourceInfo
			{
			public:
				using InfoVec = std::vector<JUserPtr<JGraphicResourceInfo>>;
			public:
				InfoVec vec[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
			public:
				std::vector<JUserPtr<JGraphicResourceInfo>>& operator[](const J_GRAPHIC_RESOURCE_TYPE type)noexcept;
				const std::vector<JUserPtr<JGraphicResourceInfo>>& operator[](const J_GRAPHIC_RESOURCE_TYPE type)const noexcept;
			};
		private:
			MultiResourceInfo info;
		protected:
			void DestroyGraphicResource(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex);
			void DestroyGraphicResource(const J_GRAPHIC_RESOURCE_TYPE rType);
			void DestroyAllTexture();
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo);
		public:
			uint GetDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			std::vector<J_GRAPHIC_RESOURCE_TYPE> GetResourceTypeVec()const noexcept final;
		protected:
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final;
			JUserPtr<JGraphicResourceInfo> GetFirstGraphicInfo()const noexcept final;
		public:
			bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			bool AllowHoldMultiHold(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
		};

		template<uint count>
		class JGraphicWideSingleResourceHolder : public JGraphicResourceInterface
		{
		private:
			JUserPtr<JGraphicResourceInfo> info[count];
		protected:
			void DestroyGraphicResource(const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				int index = GetIndex(rType);
				if (index == invalidIndex)
					return;

				JGraphicResourceInfo::Destroy(info[index].Release());
				for (int i = index; i < count - 1; ++i)
					info[i] = std::move(info[i + 1]);
			}
			void DestroyAllTexture()
			{
				for (uint i = 0; i < count; ++i)
					JGraphicResourceInfo::Destroy(info[i].Release());
			}
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo)
			{
				for (uint i = 0; i < count; ++i)
				{
					if (info[i] == nullptr)
					{
						info[i] = newInfo;
						return;
					}
				}
			}
		public:
			uint GetDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final
			{
				return GetIndex(rType) != invalidIndex ? 1 : 0;
			}
			std::vector<J_GRAPHIC_RESOURCE_TYPE> GetResourceTypeVec()const noexcept final
			{
				std::vector<J_GRAPHIC_RESOURCE_TYPE> res;
				for (uint i = 0; i < count; ++i)
				{
					if (info[i] != nullptr)
						res.push_back(info[i]->GetGraphicResourceType());
				}
				return res;
			}
		protected:
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final
			{
				int index = GetIndex(rType);
				return index != invalidIndex ? info[index] : nullptr;
			}
			JUserPtr<JGraphicResourceInfo> GetFirstGraphicInfo()const noexcept final
			{
				return info[0];
			}
		private:
			int GetIndex(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				for (uint i = 0; i < count; ++i)
				{
					if (info[i] != nullptr && info[i]->GetGraphicResourceType() == rType)
						return i;
				}
				return invalidIndex;
			}
		public:
			bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final
			{
				return GetIndex(rType) == invalidIndex;
			}
			bool HasResource(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return GetIndex(rType) != invalidIndex;
			}
			bool AllowHoldMultiHold(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return false;
			}
		};
		template<>
		class JGraphicWideSingleResourceHolder<0>
		{};

		template< uint singleCount, bool multiTypeIsFirst, J_GRAPHIC_RESOURCE_TYPE ...multiType>
		class JGraphicWideSingleAndRestrictMultiResourceHolder : public JGraphicResourceInterface
		{
		private:
			static constexpr uint multiCount = sizeof...(multiType);
			static constexpr uint canHandleResourceCount = multiCount + singleCount;
		private:
			JUserPtr<JGraphicResourceInfo> info[singleCount];
			std::vector<JUserPtr<JGraphicResourceInfo>> multiInfo[multiCount];
		protected:
			void DestroyGraphicResource(const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				int multiIndex = GetMultiVecIndex(rType);
				if (multiIndex != invalidIndex)
				{
					const uint multiVecCount = (uint)multiInfo[multiIndex].size();
					for (uint i = 0; i < multiVecCount; ++i)
						JGraphicResourceInfo::Destroy(multiInfo[multiIndex][i].Release());
					multiInfo[multiIndex].clear();
				}
				else
				{
					int index = GetSingleInfoIndex(rType);
					if (index == invalidIndex)
						return;

					JGraphicResourceInfo::Destroy(info[index].Release());
					for (int i = index; i < singleCount - 1; ++i)
						info[i] = std::move(info[i + 1]);
				}
			}
			void DestroyMultiTexture(const J_GRAPHIC_RESOURCE_TYPE rType, const uint index)
			{
				int multiIndex = GetMultiVecIndex(rType);
				if (multiIndex != invalidIndex)
				{
					if (multiInfo[multiIndex].size() <= index)
						return;

					JGraphicResourceInfo::Destroy(multiInfo[multiIndex][index].Release());
					multiInfo[multiIndex].erase(multiInfo[multiIndex].begin() + index);
				}
			}
			void DestroyAllTexture()
			{
				for (uint i = 0; i < singleCount; ++i)
					JGraphicResourceInfo::Destroy(info[i].Release());

				for (uint i = 0; i < multiCount; ++i)
				{
					const uint multiVecCount = (uint)multiInfo[i].size();
					for (uint j = 0; j < multiVecCount; ++j)
						JGraphicResourceInfo::Destroy(multiInfo[i][j].Release());
					multiInfo[i].clear();
				}
			}
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo)
			{
				int multiIndex = GetMultiVecIndex(newInfo->GetGraphicResourceType());
				if (multiIndex != invalidIndex)
					multiInfo[multiIndex].push_back(newInfo);
				else
				{
					for (uint i = 0; i < singleCount; ++i)
					{
						if (info[i] == nullptr)
						{
							info[i] = newInfo;
							return;
						}
					}
				}
			}
		public:
			uint GetDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final
			{
				return IsMultiType(rType) ? multiInfo[GetMultiVecIndex(rType)].size() : (GetSingleInfoIndex(rType) != invalidIndex ? 1 : 0);
			}
			std::vector<J_GRAPHIC_RESOURCE_TYPE> GetResourceTypeVec()const noexcept
			{
				std::vector<J_GRAPHIC_RESOURCE_TYPE> res;
				for (uint i = 0; i < multiCount; ++i)
				{
					for (const auto& data : multiInfo[i])
					{
						if (data != nullptr)
							res.push_back(data->GetGraphicResourceType());
					}
				}
				for (uint i = 0; i < singleCount; ++i)
				{
					if (info[i] != nullptr)
						res.push_back(info[i]->GetGraphicResourceType());
				}
				return res;
			}
		protected:
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final
			{
				int multiIndex = GetMultiVecIndex(rType);
				if (multiIndex != invalidIndex)
					return multiIndex != invalidIndex ? (multiInfo[multiIndex].size() > dataIndex ? multiInfo[multiIndex][dataIndex] : nullptr) : nullptr;
				else
				{
					int index = GetSingleInfoIndex(rType);
					return index != invalidIndex ? info[index] : nullptr;
				}
			}
			JUserPtr<JGraphicResourceInfo> GetFirstGraphicInfo()const noexcept final
			{
				int firstMultiVecIndex = FindFirstValidMultiVecIndex();
				if (multiTypeIsFirst && firstMultiVecIndex != invalidIndex)
					return multiInfo[firstMultiVecIndex][0];
				else
					return info[0];
			}
		private:
			int GetSingleInfoIndex(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				if (!IsMultiType(rType))
				{
					for (uint i = 0; i < singleCount; ++i)
					{
						if (info[i] != nullptr && info[i]->GetGraphicResourceType() == rType)
							return i;
					}
				}
				return invalidIndex;
			}
			int GetMultiVecIndex(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				auto findMultiVecIndexLam = [](const J_GRAPHIC_RESOURCE_TYPE vecType, const J_GRAPHIC_RESOURCE_TYPE rType, int& index, bool& isFind)
				{
					if (isFind)
						return;

					if (vecType == rType)
						isFind = true;
					else
						++index;
				};
				int index = 0;
				bool isFind = false;
				(findMultiVecIndexLam(multiType, rType, index, isFind), ...);

				return isFind ? index : invalidIndex;
			}
			int FindFirstValidMultiVecIndex()const noexcept
			{
				auto findMultiVecIndexLam = [](const J_GRAPHIC_RESOURCE_TYPE vecType, size_t size, int& index, bool& isFind)
				{
					if (isFind)
						return;

					if (size > 0)
						isFind = true;
					else
						++index;
				};
				int index = 0;
				bool isFind = false;
				(findMultiVecIndexLam(multiType, multiInfo[index].size(), index, isFind), ...);

				return isFind ? index : invalidIndex;
			}
		public:
			bool IsMultiType(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return ((rType == multiType) || ...);
			}
			bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final
			{
				return IsMultiType(rType) ? true : GetSingleInfoIndex(rType) == invalidIndex;
			}
			bool HasResource(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return IsMultiType(rType) ? GetMultiVecIndex(rType) != invalidIndex : GetSingleInfoIndex(rType) != invalidIndex;
			}
			bool AllowHoldMultiHold(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return IsMultiType(rType);
			}
		};

		template<bool multiTypeIsFirst, J_GRAPHIC_RESOURCE_TYPE ...multiType>
		class JGraphicWideSingleAndRestrictMultiResourceHolder<0, multiTypeIsFirst, multiType...>
		{};

		template<int count>
		class JGraphicMultiResourceDataIndexManager
		{
		private:
			int8 indexList[count];
		public:
			int GetIndex(const uint index)const noexcept
			{
				return static_cast<int>(indexList[index]);
			}
			void ReflectCreateResource(const uint index)noexcept
			{
				int8 max = 0;
				for (uint i = 0; i < count; ++i)
				{
					if (i == index)
						continue;
					if (max < indexList[i])
						max = indexList[i];
				}
				indexList[index] = max;
			}
			void ReflectDestroyResource(const uint index)noexcept
			{
				int8 preIndex = indexList[index];
				for (uint i = 0; i < count; ++i)
				{
					if (i == index)
						continue;

					if (indexList[i] > preIndex)
						--indexList[i];
				}
				indexList[index] = invalidIndex;
			}
		};

		using JGraphicResourceInterfacePointer = Core::JTypeImplInterfacePointer<JGraphicResourceInterface>;
		class JGraphicResourceUserInterface final
		{
		private:
			JUserPtr<JGraphicResourceInterfacePointer> gPtrWrapper = nullptr;
		public:
			JGraphicResourceUserInterface() = default;
			JGraphicResourceUserInterface(JGraphicResourceInterface* gInterface);
			~JGraphicResourceUserInterface() = default;
		public:
			uint GetDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			uint GetResourceWidth(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			uint GetResourceHeight(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			JVector2F GetResourceSize(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			JVector2F GetResourceInvSize(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			int GetResourceArrayIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			int GetResourceArrayIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept;
			int GetHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept;
			int GetOptionHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint dataIndex)const noexcept;
			uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept;
			Graphic::ResourceHandle GetGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint bIndex, const uint dataIndex) const noexcept;
			Graphic::ResourceHandle GetOptionGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint bIndex, const uint dataIndex) const noexcept;
		public:
			int GetFirstResourceArrayIndex()const noexcept;
			int GetFirstResourceHeapStart(const J_GRAPHIC_BIND_TYPE bType)const noexcept;
			JVector2F GetFirstResourceSize()const noexcept;
			JVector2F GetFirstResourceInvSize()const noexcept;
			/**
			* @return invalid index(cast J_GRAPHIC_RESOURCE_TYPE) if access invalid data
			*/
			J_GRAPHIC_RESOURCE_TYPE GetFirstResourceType()const noexcept;
			Graphic::ResourceHandle GetFirstGpuHandle(const J_GRAPHIC_BIND_TYPE bType) const noexcept;
			J_GRAPHIC_MIP_MAP_TYPE GetFirstMipmapType()const noexcept;
			int GetResourceDataIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept;
		public:
			bool IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept;
			bool HasHandle(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept;
			bool HasFirstHandle()const noexcept;
			bool HasOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint dataIndex)const noexcept;
			bool HasFirstOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)const noexcept;
		public:
			//Debug
			/**
			* @brief for debug texture mipmap
			*/
			bool TryFirstResourceMipmapBind(_Out_ std::vector<Graphic::ResourceHandle>& gpuHandle, _Out_ std::vector<Core::JDataHandle>& dataHandle)const;
			static void ClearFirstResourceMipmapBind(_Inout_ std::vector<Core::JDataHandle>& dataHandle);
		};
	}
}
