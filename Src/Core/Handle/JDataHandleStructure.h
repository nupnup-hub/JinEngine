#pragma once
#include<vector>
#include<bitset>
#include"../JDataType.h"
#include"../Pointer/JOwnerPtr.h"
#include"../Guid/GuidCreator.h"

namespace JinEngine
{
	class JObject;
	namespace Core
	{
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
			template<typename uint Capacity, typename Type> friend class JDataHandleStructure;
		private:
			int index = Handle::invalidNumber;
			size_t validNumber = Handle::invalidNumber;
			const size_t structureGuid;
		public:
			~JDataHandle() = default;
			JDataHandle(JDataHandle&& rhs) = default;
			JDataHandle& operator=(JDataHandle && rhs) = default;
		private:
			JDataHandle(const int index, const size_t validNumber, const size_t structureGuid)
				:index(index), validNumber(validNumber), structureGuid(structureGuid)
			{}
			JDataHandle(const size_t structureGuid)
				:structureGuid(structureGuid)
			{}
		public:
			bool IsValid()const noexcept
			{
				return index != Handle::invalidNumber;
			}
		};

		template<typename uint Capacity, typename Type>
		class JDataHandleStructure
		{ 
		private:
			JOwnerPtr<Type> data[Capacity];
			// 0 is empty, 1 is full
			std::bitset<Capacity> arrState;
			//magic Number Arr
			size_t arrNumber[Capacity];
			//now magic Number
			size_t validNumber = 0;
			//now valid index
			int validIndex = 0;
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
			JUserPtr<Type> GetUser(const JDataHandle& handle)const noexcept
			{
				if (IsValidHandle(handle))
					return JUserPtr<Type>{ data[handle.index] };
				else
					return JUserPtr<Type>{};
			}
		protected:
			Type* Get(const JDataHandle& handle)const noexcept
			{
				if (IsValidHandle(handle))
					return data[handle.index].Get();
				else
					return nullptr;
			}
		public:
			JDataHandle Add(Type* type)noexcept
			{
				if (!HasValidIndex() || type == nullptr)
					return CreateInvalidHandle();
				data[validIndex] = JOwnerPtr<Type>{ type };
				return SuccessProcess();
			}
			JDataHandle Add(JOwnerPtr<Type> type)noexcept
			{
				if (!HasValidIndex() || !type.IsValid())
					return CreateInvalidHandle();
				data[validIndex] = std::move(type);
				return SuccessProcess();
			}
			bool Remove(const JDataHandle& handle)noexcept
			{
				if (!IsValidHandle(handle))
					return false;

				validIndex = handle.index;
				arrState[handle.index] = Handle::empty;
				arrNumber[handle.index] = 0;
				if constexpr (std::is_base_of_v<JObject, Type>)
					JObject::BeginDestroy(data[handle.index].Get());
				data[handle.index].Clear();
				return true;
			}
			JOwnerPtr<Type> Release(const JDataHandle& handle)noexcept
			{
				if (!IsValidHandle(handle))
					return JOwnerPtr<Type>{};

				validIndex = handle.index;
				arrState[handle.index] = Handle::empty;
				arrNumber[handle.index] = 0;

				return std::move(data[handle.index]);
			}
			void Clear()
			{
				for (uint i = 0; i < Capacity; ++i)
				{
					if constexpr (std::is_base_of_v<JObject, Type>)
					{
						if(data[i].IsValid())
							JObject::BeginDestroy(data[i].Get());
					}
					data[i].Clear();
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