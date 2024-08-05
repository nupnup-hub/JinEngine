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
#include"../Image/JImageProcessingEnum.h" 
#include"../../Core/Reflection/JTypeImplBase.h"
#include"../../Core/Math/JVector.h"  
#include"../../Object/GraphicRule/GraphicResource/JGraphicModuleTextureResourceUserAccess.h"

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
		//class JCullingInterface;

		/**
		* 주로 impl class가 해당 interface을 상속한다
		* interface는 info를 소유하는 wrapper로 graphic manager와 impl간의 중간역할을 한다
		* impl이 직접 JGraphicResourceInfo를 상속하지 않은건 외부로 노출할 수 없기때문이다.
		*/
		//수정 Impl의 상위 interfacae가아닌 Graphic module내에서 관리하는 객체로 수정하고자
		//기존의 public Core::JTypeImplInterfacePointerHolder<JGraphicResourceInterface> 상속받는 코드는 수정한다.
		//2024-07-27

		class JGraphicResourceInterface : public JGraphicResourceUserInterface
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
			/**
			* @parameter newInfo is always valid user pointer and has fit space
			* @parameter dataIndex used multi holder not single holder
			*/
			virtual void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex = 0) = 0;
		public:
			uint GetResourceWidth(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final;
			uint GetResourceHeight(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final;
			JVector2F GetResourceSize(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final;
			JVector2F GetResourceInvSize(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final;
			int GetResourceArrayIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final;
			int GetResourceArrayIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
			int GetHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept final;
			int GetOptionHeapIndexStart(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint dataIndex)const noexcept final;
			uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint dataIndex)const noexcept final;
			uint GetViewCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
			ResourceHandle GetGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const uint bIndex, const uint dataIndex) const noexcept final;
			ResourceHandle GetOptionGpuHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const uint bIndex, const uint dataIndex) const noexcept final;
		public:
			int GetFirstResourceArrayIndex()const noexcept final;
			int GetFirstResourceHeapStart(const J_GRAPHIC_BIND_TYPE bType)const noexcept final;
			JVector2F GetFirstResourceSize()const noexcept final;
			JVector2F GetFirstResourceInvSize()const noexcept final;
			J_GRAPHIC_RESOURCE_TYPE GetFirstResourceType()const noexcept final;
			ResourceHandle GetFirstGpuHandle(const J_GRAPHIC_BIND_TYPE bType) const noexcept final;
			J_GRAPHIC_MIP_MAP_TYPE GetFirstMipmapType()const noexcept final;  
			//for debugging
			virtual std::vector<J_GRAPHIC_RESOURCE_TYPE> GetResourceTypeVec()const noexcept = 0;
		protected:
			virtual JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept = 0;
			virtual JUserPtr<JGraphicResourceInfo> GetFirstGraphicInfo()const noexcept = 0;
		public:
			bool IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final; 
			bool IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
			bool HasFirstHandle()const noexcept final;
			bool HasOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
			bool HasFirstOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)const noexcept final;
			bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
			virtual bool HasFixedSpaceByTaskType() const noexcept = 0; 
		protected:
			int NextResourceIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept;
		public:
			//Debug
			/**
			* @brief for debug texture mipmap
			*/
			bool TryFirstResourceMipmapBind(_Out_ std::vector<ResourceHandle>& gpuHandle, _Out_ std::vector<Core::JDataHandle>& dataHandle)const final;
			void ClearFirstResourceMipmapBind(_Inout_ std::vector<Core::JDataHandle>& dataHandle)final;
		};
		class JGraphicSingleResourceHolder : public JGraphicResourceInterface
		{
		private:
			JUserPtr<JGraphicResourceInfo> info = nullptr;
		public:
			bool DestroyGraphicResource();
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex);
		public:
			uint GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;  
			std::vector<J_GRAPHIC_RESOURCE_TYPE> GetResourceTypeVec()const noexcept final;
		protected:
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final;
			JUserPtr<JGraphicResourceInfo> GetFirstGraphicInfo()const noexcept final;
		public: 
			bool HasGraphicResourceHandle()const noexcept;
			bool HasFixedSpaceByTaskType() const noexcept final; 
		};
		class JGraphicTypePerSingleResourceHolder : public JGraphicResourceInterface
		{
		private:
			JUserPtr<JGraphicResourceInfo> info[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
		protected:
			void DestroyGraphicResource(const J_GRAPHIC_RESOURCE_TYPE rType);
			void DestroyAllTexture();
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex);
		public:
			uint GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;   
			uint GetAllocableResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
			std::vector<J_GRAPHIC_RESOURCE_TYPE> GetResourceTypeVec()const noexcept final;
		protected:
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final;
			JUserPtr<JGraphicResourceInfo> GetFirstGraphicInfo()const noexcept final;
		public:  
			bool HasFixedSpaceByTaskType()const noexcept final; 
		};
		class JGraphicMultiResourceHolder : public JGraphicResourceInterface
		{
		//private:
		//	friend class JCullingInterface;
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
		public:
			void DestroyGraphicResource(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex);
			void DestroyGraphicResource(const J_GRAPHIC_RESOURCE_TYPE rType);
			void DestroyAllTexture();
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex);
		public:
			uint GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;  
			uint GetAllocableResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
			std::vector<J_GRAPHIC_RESOURCE_TYPE> GetResourceTypeVec()const noexcept final;
		protected:
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final;
			JUserPtr<JGraphicResourceInfo> GetFirstGraphicInfo()const noexcept final;
		public:  
			bool HasFixedSpaceByTaskType() const noexcept final;
		};
 
		/*
		* type당 하나의 resource가필요할경우 사용.
		*/
		template<typename TypeSequence>
		class JGraphicWideSingleResourceHolder : public JGraphicResourceInterface
		{
			static_assert(std::is_same_v<TypeSequence::ValueType, J_GRAPHIC_RESOURCE_TYPE>, "Invalid TypeSequence");
		private:
			static constexpr uint count = TypeSequence::count;
		private:
			static int order[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
		private:
			JUserPtr<JGraphicResourceInfo> info[count];
		public:
			JGraphicWideSingleResourceHolder()
			{
				static bool initTrigger = false;
				if (!initTrigger)
				{
					TypeSequence::StuffTypeSequenceOrder(std::make_index_sequence<count>(), order, (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT);
					initTrigger = true;
				}
			}
		protected:
			void DestroyGraphicResource(const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				int index = GetTypeIndex(rType);
				if (index == invalidIndex || info[index] == nullptr)
					return;

				JGraphicResourceInfo::Destroy(info[index].Release()); 
			}
			void DestroyAllTexture()
			{
				for (uint i = 0; i < count; ++i)
					JGraphicResourceInfo::Destroy(info[i].Release());
			}
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex)
			{
				int index = order[(uint)newInfo->GetGraphicResourceType()];
				info[index] = newInfo; 
			}
		public:
			uint GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final
			{
				//return ((rType == type ? GetGraphicInfo(type) != nullptr : 0) + ...);
				return uint(GetGraphicInfo(rType, 0) != nullptr);
			} 
			uint GetAllocableResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final
			{
				return GetTypeIndex(rType) != invalidIndex ? 1 : 0;
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
				int index = GetTypeIndex(rType);
				return index != invalidIndex ? info[index] : nullptr;
			}
			JUserPtr<JGraphicResourceInfo> GetFirstGraphicInfo()const noexcept final
			{
				for (uint i = 0; i < count; ++i)
				{
					if (info[i] != nullptr)
						return info[i];
				}
				return nullptr;
			}
		private:
			int GetTypeIndex(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{	
				return order[(uint)rType];
			}
		public: 
			bool HasFixedSpaceByTaskType() const noexcept final
			{
				return false;
			} 
		}; 
		template<typename TypeSequence>
		int JGraphicWideSingleResourceHolder<TypeSequence>::order[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT]; 

		/*
		* type당 복수의 resource가필요하며
		* 각각이 다른 task에 사용될때 유용한 holder
		* 하위 class에서 GetAllocableResourceCount을 정의 해야한다.
		*/
		template<typename StaticTupleSequence>
		class JGraphicRestrictMultiResourceHolder : public JGraphicResourceInterface
		{
		public:
			using FirstType = typename StaticTupleSequence::FirstType;
			using SecondType = typename StaticTupleSequence::SecondType;
		private: 
			static constexpr uint count = StaticTupleSequence::count; 
			static constexpr bool typeIsFirst = std::is_same_v<FirstType, J_GRAPHIC_RESOURCE_TYPE>;
			static constexpr bool hasType = (std::is_same_v<FirstType, J_GRAPHIC_RESOURCE_TYPE> || std::is_same_v<SecondType, J_GRAPHIC_RESOURCE_TYPE>);
			static constexpr bool hasIndex = (std::is_integral_v<FirstType> || std::is_integral_v<SecondType>);
			static constexpr bool isValid = hasType && hasIndex;		 
		private: 
			static_assert(isValid, "Invalid StaticTupleSequence");
		private:  
			static int order[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
			static uint arrayLength[count];
		private: 
			JUserPtr<JGraphicResourceInfo>* info[count];
		public:
			JGraphicRestrictMultiResourceHolder()
			{
				static bool initTrigger = false;
				if (!initTrigger)
				{
					for (uint i = 0; i < count; ++i)
						order[i] = invalidIndex;

					if constexpr (typeIsFirst)
					{
						StaticTupleSequence:: template StuffTypeSequenceOrder<FirstType>(std::make_index_sequence<count>(), order, (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT);
						for (uint i = 0; i < count; ++i)
							arrayLength[i] = StaticTupleSequence::template At<SecondType>(i);
					}
					else
					{
						StaticTupleSequence::template StuffTypeSequenceOrder<SecondType>(std::make_index_sequence<count>(), order, (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT);
						for (uint i = 0; i < count; ++i)
							arrayLength[i] = StaticTupleSequence::template At<FirstType>(i);
					}
					initTrigger = true;
				}
				for (uint i = 0; i < count; ++i)
					info[i] = new JUserPtr<JGraphicResourceInfo>[arrayLength[i]]();
			}
			~JGraphicRestrictMultiResourceHolder()
			{
				for (uint i = 0; i < count; ++i)
					delete[] info[i];
			}
		public:
			void DestroyGraphicResource(const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				const int typeIndex = GetTypeIndex(rType);
				if (typeIndex == invalidIndex)
					return;

				JUserPtr<JGraphicResourceInfo>* rArray = info[typeIndex];
				for (uint i= 0; i < arrayLength[typeIndex]; ++i)
				{
					if(rArray[i] != nullptr)
						JGraphicResourceInfo::Destroy(rArray[i].Release());
				}
			} 
			void DestroyAllTexture()
			{
				for (uint i = 0; i < count; ++i)
				{
					for (uint j = 0; j < arrayLength[j]; ++j)
					{
						if (info[i][j] == nullptr)
							continue;

						JGraphicResourceInfo::Destroy(info[i][j].Release());
					}
				}
			}
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex)
			{  
				JUserPtr<JGraphicResourceInfo>* rArray = GetArray(newInfo->GetGraphicResourceType());
				rArray[dataIndex] = newInfo;
			}
		public:
			uint GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final
			{ 
				return GetTypeIndex(rType) != invalidIndex ? arrayLength[GetTypeIndex(rType)] : 0;
			}
			std::vector<J_GRAPHIC_RESOURCE_TYPE> GetResourceTypeVec()const noexcept
			{
				std::vector<J_GRAPHIC_RESOURCE_TYPE> res;
				for (uint i = 0; i < count; ++i)
				{
					for (uint j = 0; j < arrayLength[j]; ++j)
					{
						if (info[i][j] != nullptr)
							res.push_back(info[i][j]->GetGraphicResourceType());
					}
				} 
				return res;
			}
		protected:
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final
			{
				const int typeIndex = GetTypeIndex(rType);
				return typeIndex != invalidIndex && arrayLength[typeIndex] > dataIndex ? info[typeIndex][dataIndex]: nullptr;
			}
			JUserPtr<JGraphicResourceInfo> GetFirstGraphicInfo()const noexcept final
			{ 
				for (uint i = 0; i < count; ++i)
				{ 
					for (uint j = 0; j < arrayLength[j]; ++j)
					{
						if (info[i][j] != nullptr)
							return info[i][j];
					}
				} 
				return nullptr;
			}		
		private:
			int GetTypeIndex(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return order[(uint)rType];
			}  
			JUserPtr<JGraphicResourceInfo>* GetArray(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return GetTypeIndex(rType) != invalidIndex ? info[(uint)GetTypeIndex(rType)] : nullptr;
			}
		public:  
			bool HasFixedSpaceByTaskType() const noexcept final
			{
				return true;
			} 
		};	
		template<typename StaticTupleSequence>
		int JGraphicRestrictMultiResourceHolder<StaticTupleSequence>::order[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
		template<typename StaticTupleSequence>
		uint JGraphicRestrictMultiResourceHolder<StaticTupleSequence>::arrayLength[JGraphicRestrictMultiResourceHolder::count];
	 
		/*
		* type당 복수의 resource가필요하며
		* 각각 갯수의 제한을 알 수 없는 경우 사용.
		*/
		template<typename SignleTypeSequence, typename MultiTypeSequence, bool multiTypeIsFirst>
		class JGraphicWideSingleAndMultiResourceHolder : public JGraphicResourceInterface
		{
		private:
			static constexpr uint singleCount = SignleTypeSequence::count;
			static constexpr uint multiCount = MultiTypeSequence::count;  
		private:
			static int singleOrder[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT]; 
			static int multiOrder[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
		private:
			JUserPtr<JGraphicResourceInfo> singleInfo[singleCount];
			std::vector<JUserPtr<JGraphicResourceInfo>> multiInfo[multiCount];
		public:
			JGraphicWideSingleAndMultiResourceHolder()
			{
				static bool initTrigger = false;
				if (!initTrigger)
				{
					SignleTypeSequence::StuffTypeSequenceOrder(std::make_index_sequence<singleCount>(), singleOrder, (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT);
					MultiTypeSequence::StuffTypeSequenceOrder(std::make_index_sequence<multiCount>(), multiOrder, (uint)J_GRAPHIC_RESOURCE_TYPE::COUNT);
					initTrigger = true;
				} 
			}
			~JGraphicWideSingleAndMultiResourceHolder()
			{

			}
		public:
			void DestroyGraphicResource(const J_GRAPHIC_RESOURCE_TYPE rType)
			{
				int multiIndex = GetMultiInfoVecIndex(rType);
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
					if (index == invalidIndex || singleInfo[index] == nullptr)
						return;

					JGraphicResourceInfo::Destroy(singleInfo[index].Release());
				}
			}
			void DestroyMultiTexture(const J_GRAPHIC_RESOURCE_TYPE rType, const uint index)
			{
				int multiIndex = GetMultiInfoVecIndex(rType);
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
					JGraphicResourceInfo::Destroy(singleInfo[i].Release());

				for (uint i = 0; i < multiCount; ++i)
				{
					const uint multiVecCount = (uint)multiInfo[i].size();
					for (uint j = 0; j < multiVecCount; ++j)
						JGraphicResourceInfo::Destroy(multiInfo[i][j].Release());
					multiInfo[i].clear();
				}
			}
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex)
			{
				int singleIndex = GetSingleInfoIndex(newInfo->GetGraphicResourceType());
				int multiIndex = GetMultiInfoVecIndex(newInfo->GetGraphicResourceType());
				if (multiIndex != invalidIndex)
					multiInfo[multiIndex].push_back(newInfo);
				else if (singleInfo[singleIndex] != nullptr)
					singleInfo[singleIndex] = newInfo;
			}
		public:
			uint GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final
			{
				return IsMultiType(rType) ? (uint)multiInfo[GetMultiInfoVecIndex(rType)].size() : (HasSingleInfo(rType) ? 1 : 0);
			}
			uint GetAllocableResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final
			{
				return IsMultiType(rType) ? UINT_MAX : (GetSingleInfoIndex(rType) != invalidIndex ? 1 : 0);
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
					if (singleInfo[i] != nullptr)
						res.push_back(singleInfo[i]->GetGraphicResourceType());
				}
				return res;
			}
		protected:
			/**
			* @brief Unsafe so using only multi type case
			*/
			uint GetMultiDataCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return  multiInfo[GetMultiInfoVecIndex(rType)].size();
			}
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final
			{
				int multiIndex = GetMultiInfoVecIndex(rType);
				if (multiIndex != invalidIndex)
					return (multiInfo[multiIndex].size() > dataIndex ? multiInfo[multiIndex][dataIndex] : nullptr);
				else
				{
					int index = GetSingleInfoIndex(rType);
					return index != invalidIndex ? singleInfo[index] : nullptr;
				}
			}
			JUserPtr<JGraphicResourceInfo> GetFirstGraphicInfo()const noexcept final
			{
				if (multiTypeIsFirst)
				{
					int firstMultiVecIndex = FindFirstValidMultiVecIndex();
					if (firstMultiVecIndex != invalidIndex)
						return multiInfo[firstMultiVecIndex][0];
					for (uint i = 0; i < singleCount; ++i)
					{
						if (singleInfo[i] != nullptr)
							return singleInfo[i];
					}
					return nullptr;
				}
				else
				{
					for (uint i = 0; i < singleCount; ++i)
					{
						if (singleInfo[i] != nullptr)
							return singleInfo[i];
					}
					int firstMultiVecIndex = FindFirstValidMultiVecIndex();
					if (firstMultiVecIndex != invalidIndex)
						return multiInfo[firstMultiVecIndex][0];
					return nullptr;
				}
			}
		private:
			int GetSingleInfoIndex(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return singleOrder[(uint)rType];
			}
			int GetMultiInfoVecIndex(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return multiOrder[(uint)rType];
			}
			int FindFirstValidMultiVecIndex()const noexcept
			{
				for (uint i = 0; i < multiCount; ++i)
				{
					if (multiInfo[i].size() > 0)
						return i;
				}
				return invalidIndex;
			}
		public:
			bool IsMultiType(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return GetMultiInfoVecIndex(rType) != invalidIndex;
			}
			bool HasFixedSpaceByTaskType() const noexcept final
			{
				return false;
			} 
		private:
			bool HasSingleInfo(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return GetSingleInfoIndex(rType) != invalidIndex && singleInfo[GetSingleInfoIndex(rType)] != nullptr;
			}
			bool HasMultiInfo(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept
			{
				return GetMultiInfoVecIndex(rType) != invalidIndex && multiInfo[GetMultiInfoVecIndex(rType)].size() > 0;
			}
		};
		template<typename SignleTypeSequence, typename MultiTypeSequence, bool multiTypeIsFirst>
		int JGraphicWideSingleAndMultiResourceHolder<SignleTypeSequence, MultiTypeSequence, multiTypeIsFirst>::singleOrder[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
		template<typename SignleTypeSequence, typename MultiTypeSequence, bool multiTypeIsFirst>
		int JGraphicWideSingleAndMultiResourceHolder<SignleTypeSequence, MultiTypeSequence, multiTypeIsFirst>::multiOrder[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];

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
	}
}
