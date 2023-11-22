#pragma once
#include<vector>
#include<bitset>
#include"../JCoreEssential.h"
#include"../Pointer/JOwnerPtr.h"
#include"../Guid/JGuidCreator.h"
 
namespace JinEngine
{
	namespace Core
	{
		class JIdentifier;
		namespace Handle
		{
			static constexpr int invalidNumber = -1;
			static constexpr size_t maxValidNumber = SIZE_MAX;
			static constexpr uint8 empty = 0;
			static constexpr uint8 full = 1;
		}

		class JDataHandle
		{
		private:
			template<typename uint Capacity, typename Type, bool> friend class JDataHandleStructure;
		private:
			const size_t structureGuid;
			size_t validNumber = Handle::invalidNumber;		//magic Number
		private:
			const uint structureIndex = 0;				//for multi dimension structure
			int index = Handle::invalidNumber;			//data index
		public:
			~JDataHandle() = default;
			JDataHandle(const JDataHandle& rhs) = delete;
			JDataHandle& operator=(const JDataHandle& rhs) = delete;
			JDataHandle& operator=(JDataHandle&& rhs) = delete;
			JDataHandle(JDataHandle&& rhs)noexcept
				:index(rhs.index),
				validNumber(rhs.validNumber),
				structureGuid(rhs.structureGuid),
				structureIndex(rhs.structureIndex)
			{ 
				rhs.Clear();  
			}
		private:
			JDataHandle(const int index, const size_t validNumber, const size_t structureGuid, const uint structureIndex= 0)
				:index(index), validNumber(validNumber), structureGuid(structureGuid), structureIndex(structureIndex)
			{}
			JDataHandle(const size_t structureGuid, const uint structureIndex = 0)
				:structureGuid(structureGuid), structureIndex(structureIndex)
			{}
		public:
			bool IsValid()const noexcept
			{
				return index != Handle::invalidNumber;
			}
		private:
			void Clear()
			{
				index = Handle::invalidNumber;
				validNumber = Handle::invalidNumber;
			}
		};

		template<typename uint Capacity, typename Type, bool useOwnerPtr = true>
		class JDataHandleStructure final
		{
		private:
			template<bool isOwner>
			struct PointerTypeDetermine
			{
			public:
				using PointerType = JUserPtr<Type>;
			};
			template<>
			struct PointerTypeDetermine<true>
			{
			public:
				using PointerType = JOwnerPtr<Type>;
			};
		private:
			using PointerType = typename PointerTypeDetermine<useOwnerPtr>::PointerType;
		private:
			PointerType data[Capacity];
			std::bitset<Capacity> arrState;		// 0 is empty, 1 is full
			size_t arrNumber[Capacity];			//magic Number Arr
			size_t validNumber = 0;				//now magic Number
			int validIndex = 0;					//now valid index
		private:
			const size_t guid;
		public:
			JDataHandleStructure()
				:guid(MakeGuid())
			{
				if (Capacity == 0)
					validIndex = Handle::invalidNumber;
			}
		public:
			Type* Get(const JDataHandle& handle)const noexcept
			{
				if (IsValidHandle(handle))
					return Get(handle.index);
				else
					return nullptr;
			} 
			int GetValidIndex()const noexcept
			{
				return validIndex;
			}
			int GetEmptyCount()const noexcept
			{ 
				return arrState.size() - arrState.count();
			}
		protected:
			Type* Get(const uint index)const noexcept
			{
				return data[index].Get();
			}
			void ClearData(const uint index)
			{
				data[index].Clear();
			}
		public:
			JDataHandle Add(Type* type)noexcept
			{
				if (!HasValidIndex() || type == nullptr)
					return CreateInvalidHandle();
				data[validIndex] = PointerType{ type };
				return SuccessProcess();
			}
			JDataHandle Add(PointerType&& type)noexcept
			{
				if (!HasValidIndex() || !type.IsValid())
					return CreateInvalidHandle();
				data[validIndex] = std::move(type);
				return SuccessProcess();
			}
			bool Remove(JDataHandle& handle)noexcept
			{ 
				if (!IsValidHandle(handle))
					return false;

				validIndex = handle.index;
				arrState[handle.index] = Handle::empty;
				arrNumber[handle.index] = 0;
				if constexpr (std::is_base_of_v<JIdentifier, Type>)
					JIdentifier::BeginDestroy(Get(handle.index));
				ClearData(handle.index);
				handle.Clear();
				return true;
			}
			PointerType Release(JDataHandle& handle)noexcept
			{
				if (!IsValidHandle(handle))
					return PointerType{};

				validIndex = handle.index;
				arrState[handle.index] = Handle::empty;
				arrNumber[handle.index] = 0;
				const int index = handle.index;
				handle.Clear();
				return std::move(data[index]);
			}
			void Clear()
			{
				for (uint i = 0; i < Capacity; ++i)
				{
					if constexpr (std::is_base_of_v<JIdentifier, Type>)
					{
						if (data[i] != nullptr)
							JIdentifier::BeginDestroy(Get(i));
					}
					ClearData(i); 
					arrState[i] = 0;
					arrNumber[i] = 0;
				}
				validNumber = 0;
				validIndex = 0;
			}
		public:
			JDataHandle CreateInvalidHandle()const noexcept
			{
				return JDataHandle{ guid };
			}
			void PushInvalidHandle(std::vector<JDataHandle>& handleVec)const noexcept
			{
				handleVec.push_back(CreateInvalidHandle());
			}
			void PushValidHandle(JDataHandle& from, std::vector<JDataHandle>& to)
			{
				JDataHandle newHandle = CreateInvalidHandle();
				TransitionHandle(from, newHandle);
				to.push_back(std::move(newHandle));
			}
			void TransitionHandle(JDataHandle& from, JDataHandle& to)
			{
				if (from.structureGuid == guid && from.structureGuid == to.structureGuid)
				{
					if (to.IsValid())
						Remove(to);

					to.index = from.index;
					to.validNumber = from.validNumber;

					from.index = Handle::invalidNumber;
					from.validNumber = Handle::invalidNumber;
				}
			}
		public:
			bool IsValidHandle(const JDataHandle& handle)const noexcept
			{
				return (handle.structureGuid == guid && handle.IsValid() && arrState[handle.index]) &&
					arrNumber[handle.index] == handle.validNumber;
			}
			bool CanAdd()const noexcept
			{
				return HasValidIndex();
			}
		private:
			JDataHandle SuccessProcess() noexcept
			{
				arrState[validIndex] = Handle::full;
				arrNumber[validIndex] = validNumber;

				JDataHandle res{ validIndex , validNumber, guid };
				validIndex = FindNextValidIndex();
				++validNumber;
				if (Handle::maxValidNumber == validNumber)
					validNumber = 0;
				return res;
			}
		private:
			bool HasValidIndex()const noexcept
			{
				return validIndex != Handle::invalidNumber;
			}
			int FindNextValidIndex()const noexcept
			{
				for (uint i = 0; i < Capacity; ++i)
				{
					if (!arrState[i])
						return i;
				}
				return Handle::invalidNumber;
			}
		};
	}
}