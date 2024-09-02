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
#include"../../Core/Func/Functor/JFunctor.h"
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
		public:
			using DestoryInfoF = Core::JSFunctorType<void, JGraphicResourceInfo*>::Functor;
		public:
			/**
			* @parameter newInfo is always valid user pointer and has fit space
			* @parameter dataIndex used multi holder not single holder
			*/
			virtual void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex = 0) = 0;
			virtual void RemoveInfo(DestoryInfoF& destroyF, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType, const uint localIndex = 0) = 0;
			virtual void RemoveInfoOfType(DestoryInfoF& destroyF, const J_GRAPHIC_RESOURCE_TYPE rType) = 0;
			virtual void RemoveInfoAll(DestoryInfoF& destroyF) = 0;
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
			uint GetMipmapCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
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
			int GetResourceIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType, const uint localIndex = 0)const noexcept final;
			//for debugging
			virtual std::vector<J_GRAPHIC_RESOURCE_TYPE> GetResourceTypeVec()const noexcept = 0;
		public:
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType, const uint localIndex = 0)const noexcept;
			virtual JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const int dataIndex)const noexcept = 0;
			virtual JUserPtr<JGraphicResourceInfo> GetFirstGraphicInfo()const noexcept = 0;
		public:
			bool IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const uint dataIndex)const noexcept final;
			bool IsValidHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
			bool HasFirstHandle()const noexcept final;
			bool HasOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
			bool HasFirstOption(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_RESOURCE_OPTION_TYPE opType)const noexcept final;
			bool HasSpace(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
			virtual bool HasFixedSpaceByTaskType() const noexcept = 0;
		public:
			int NextResourceIndex(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept;;
		};

		class JGraphicSingleResourceHolder : public JGraphicResourceInterface
		{
		private:
			JUserPtr<JGraphicResourceInfo> info = nullptr;
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex)final;
			void RemoveInfo(DestoryInfoF& destroyF, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType, const uint localIndex = 0)final;
			void RemoveInfoOfType(DestoryInfoF& destroyF, const J_GRAPHIC_RESOURCE_TYPE rType) final;
			void RemoveInfoAll(DestoryInfoF& destroyF)final;
		public:
			uint GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			std::vector<J_GRAPHIC_RESOURCE_TYPE> GetResourceTypeVec()const noexcept final;
		protected:
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const int dataIndex)const noexcept final;
			JUserPtr<JGraphicResourceInfo> GetFirstGraphicInfo()const noexcept final;
		public:
			bool HasGraphicResourceHandle()const noexcept;
			bool HasFixedSpaceByTaskType() const noexcept final;
		};
		class JGraphicTypePerSingleResourceHolder : public JGraphicResourceInterface
		{
		private:
			JUserPtr<JGraphicResourceInfo> info[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex);
			void RemoveInfo(DestoryInfoF& destroyF, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType, const uint localIndex = 0)final;
			void RemoveInfoOfType(DestoryInfoF& destroyF, const J_GRAPHIC_RESOURCE_TYPE rType) final;
			void RemoveInfoAll(DestoryInfoF& destroyF)final;
		public:
			uint GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			uint GetAllocableResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
			std::vector<J_GRAPHIC_RESOURCE_TYPE> GetResourceTypeVec()const noexcept final;
		protected:
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const int dataIndex)const noexcept final;
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
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex);
			void RemoveInfo(DestoryInfoF& destroyF, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType, const uint localIndex = 0)final;
			void RemoveInfoOfType(DestoryInfoF& destroyF, const J_GRAPHIC_RESOURCE_TYPE rType) final;
			void RemoveInfoAll(DestoryInfoF& destroyF)final;
		public:
			uint GetResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType)const noexcept final;
			uint GetAllocableResourceCount(const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
			std::vector<J_GRAPHIC_RESOURCE_TYPE> GetResourceTypeVec()const noexcept final;
		protected:
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const int dataIndex)const noexcept final;
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
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex)
			{
				int index = order[(uint)newInfo->GetGraphicResourceType()];
				info[index] = newInfo;
			}
			void RemoveInfo(DestoryInfoF& destroyF, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType, const uint localIndex = 0)final
			{
				int index = order[(uint)rType]; 
				if (index == invalidIndex || info[index] == nullptr)
					return;
				 
				destroyF(info[index].Release());
			}
			void RemoveInfoOfType(DestoryInfoF& destroyF, const J_GRAPHIC_RESOURCE_TYPE rType) final
			{
				int index = order[(uint)rType];
				if (index == invalidIndex || info[index] == nullptr)
					return;

				destroyF(info[index].Release());
			}
			void RemoveInfoAll(DestoryInfoF& destroyF)final
			{ 
				for (uint i = 0; i < count; ++i)
				{
					if (info[i] != nullptr)
						destroyF(info[i].Release());
				} 
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
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const int dataIndex)const noexcept final
			{
				if (dataIndex == invalidIndex)
					return nullptr;

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
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex)
			{ 
				JUserPtr<JGraphicResourceInfo>* rArray = GetArray(newInfo->GetGraphicResourceType());
				rArray[dataIndex] = newInfo;
			}
			void RemoveInfo(DestoryInfoF& destroyF, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType, const uint localIndex = 0)final
			{
				auto infoPtr = GetGraphicInfoHandle(rType, GetResourceIndex(rType, taskType, localIndex));
				if (infoPtr == nullptr)
					return;
				 
				destroyF(infoPtr->Release());
			}
			void RemoveInfoOfType(DestoryInfoF& destroyF, const J_GRAPHIC_RESOURCE_TYPE rType) final
			{ 
				const int typeIndex = GetTypeIndex(rType);
				if (typeIndex == invalidIndex)
					return;

				JUserPtr<JGraphicResourceInfo>* rArray = info[typeIndex];
				for (uint i = 0; i < arrayLength[typeIndex]; ++i)
					destroyF(rArray[i].Release());		 
			}
			void RemoveInfoAll(DestoryInfoF& destroyF)final
			{
				std::vector<JGraphicResourceInfo*> release;
				for (uint i = 0; i < count; ++i)
				{
					for (uint j = 0; j < arrayLength[j]; ++j)
						destroyF(info[i][j].Release());
				} 
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
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const int dataIndex)const noexcept final
			{
				if (dataIndex == invalidIndex)
					return nullptr;

				const int typeIndex = GetTypeIndex(rType);
				return typeIndex != invalidIndex && arrayLength[typeIndex] > dataIndex ? info[typeIndex][dataIndex] : nullptr;
			}
			JUserPtr<JGraphicResourceInfo>* GetGraphicInfoHandle(const J_GRAPHIC_RESOURCE_TYPE rType, const int dataIndex)noexcept
			{
				const int typeIndex = GetTypeIndex(rType);
				return typeIndex != invalidIndex && arrayLength[typeIndex] > dataIndex ? &info[typeIndex][dataIndex] : nullptr;
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
		private:
			void AddInfo(const JUserPtr<JGraphicResourceInfo>& newInfo, const uint dataIndex)
			{
				int singleIndex = GetSingleInfoIndex(newInfo->GetGraphicResourceType());
				int multiIndex = GetMultiInfoVecIndex(newInfo->GetGraphicResourceType());
				if (multiIndex != invalidIndex)
					multiInfo[multiIndex].push_back(newInfo);
				else if (singleIndex != invalidIndex)
					singleInfo[singleIndex] = newInfo;
			}
			void RemoveInfo(DestoryInfoF& destroyF, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE taskType, const uint localIndex = 0)final
			{
				int multiIndex = GetMultiInfoVecIndex(rType);
				int index = GetResourceIndex(rType, taskType, localIndex);
				if (multiIndex != invalidIndex)
				{
					if (multiInfo[multiIndex].size() <= index)
						return;

					destroyF(multiInfo[multiIndex][index].Release());
					multiInfo[multiIndex].erase(multiInfo[multiIndex].begin() + index);
				}
				else
				{
					int singleIndex = GetSingleInfoIndex(rType);
					if (index == invalidIndex || singleInfo[singleIndex] == nullptr)
						return;

					destroyF(singleInfo[singleIndex].Release()); 
				}
			}
			void RemoveInfoOfType(DestoryInfoF& destroyF, const J_GRAPHIC_RESOURCE_TYPE rType) final
			{
				int multiIndex = GetMultiInfoVecIndex(rType);
				if (multiIndex != invalidIndex)
				{ 
					const uint multiVecCount = (uint)multiInfo[multiIndex].size();
					for (uint i = 0; i < multiVecCount; ++i)
						destroyF(multiInfo[multiIndex][i].Release());
					 
					multiInfo[multiIndex].clear(); 
				}
				else
				{
					int singleIndex = GetSingleInfoIndex(rType);
					if (singleIndex == invalidIndex)
						return;

					destroyF(singleInfo[singleIndex].Release());
				}
			}
			void RemoveInfoAll(DestoryInfoF& destroyF)final
			{
				std::vector<JGraphicResourceInfo*> result;
				for (uint i = 0; i < singleCount; ++i)
					destroyF(singleInfo[i].Release());

				for (uint i = 0; i < multiCount; ++i)
				{
					const uint multiVecCount = (uint)multiInfo[i].size();
					for (uint j = 0; j < multiVecCount; ++j)
						destroyF(multiInfo[i][j].Release());
					multiInfo[i].clear();
				}
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
			JUserPtr<JGraphicResourceInfo> GetGraphicInfo(const J_GRAPHIC_RESOURCE_TYPE rType, const int dataIndex)const noexcept final
			{
				if (dataIndex == invalidIndex)
					return nullptr;

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
