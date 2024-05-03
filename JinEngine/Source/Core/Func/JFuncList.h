#pragma once 
#include"Callable/JCallable.h"
#include<vector>
#include<memory>
namespace JinEngine
{
	namespace Core
	{
		//enum value has to separate one by one
		template<uint count, typename Type, typename ...Param>
		class JFuncList
		{
		protected:
			using JFuncCallable = JCallableInterface<void, Type*, Param...>;  
			using JCondCallable = JCallableInterface<bool>; 
		private:
			struct Data
			{
			public:
				std::unique_ptr<JFuncCallable> func = nullptr;
				std::unique_ptr<JCondCallable> cond = nullptr;
			public:
				Data() = default;
				Data(std::unique_ptr<JFuncCallable>&& func)
					:func(std::move(func))
				{} 
				Data(std::unique_ptr<JFuncCallable>&& func, std::unique_ptr<JCondCallable>&& cond)
					:func(std::move(func)), cond(std::move(cond))
				{}
			};
		private:
			Data list[count];
		public:
			JFuncList() = default;
		public:
			void Invoke(const uint index, Type* type, Param&&... value)
			{
				if (list[index].cond != nullptr && !(*list[index].cond)(type))
					return;

				(*list[index].func)(type, type, std::forward<Param>(value)...);
			}
			void InvokePassCondition(const uint index, Type* type, Param&&... value)
			{
				(*list[index].func)(type, type, std::forward<Param>(value)...);
			}
			void InvokeAll(Type* type, const bool doConditionTest, Param&&... value)
			{
				for (uint i = 0; i < count; ++i)
				{
					if (Has(i))
					{
						if (doConditionTest)
							Invoke(i, type, std::forward<Param>(value)...);
						else
							InvokePassCondition(i, type, std::forward<Param>(value)...);
					}
				}
			}
			void InvokeAllReverse(Type* type, const bool doConditionTest, Param&&... value)
			{ 
				for (uint i = 0; i < count; ++i)
				{
					int index = count - i - 1;
					if (Has(index))
					{
						if (doConditionTest)
							Invoke(index, type, std::forward<Param>(value)...);
						else
							InvokePassCondition(index, type, std::forward<Param>(value)...);
					}
				}
			}
		public:
			bool Has(const uint index)
			{
				return list[index].func != nullptr;
			}
		public:
			void Register(std::unique_ptr<JFuncCallable>&& func, const uint index)
			{
				list[index] = Data(std::move(func));
			}
			void Register(std::unique_ptr<JFuncCallable>&& func, std::unique_ptr<JCondCallable>&& cond, const uint index)
			{
				list[index] = Data(std::move(func), std::move(cond));
			} 
			void DeRegister(const uint index)
			{
				list[index] = nullptr;
			}
		};
		 
		template<uint funcCount, uint groupCount, typename Type, typename ...Param>
		class JFuncListG : public JFuncList<funcCount, Type, Param...>
		{  
		private:
			using Parent = typename JFuncList<funcCount, Type, Param...>;
		private:
			std::vector<uint> innerGroup[groupCount];
		public:
			void InvokeGroup(const uint index, Type* type, Param&&... value)
			{
				const uint count = (uint)innerGroup[index].size();
				for (uint i = 0; i < count; ++i)
				{
					if(Parent::Has(i))
						Parent::Invoke(innerGroup[index][i], type, std::forward<Param>(value)...);
				}
			} 
			void InvokeGroupReverse(const uint index, Type* type, Param&&... value)
			{
				const uint count = (uint)innerGroup[index].size();
				for (uint i = 0; i < count; ++i)
				{
					int innerIndex = count - i - 1;
					if (Parent::Has(innerIndex))
						Parent::Invoke(innerGroup[index][innerIndex], type, std::forward<Param>(value)...);
				}
			}
			void InvokeGroupPassCondition(const uint index, Type* type, Param&&... value)
			{
				const uint count = (uint)innerGroup[index].size();
				for (uint i = 0; i < count; ++i)
				{
					if (Parent::Has(i))
						Parent::InvokePassCondition(innerGroup[index][i], type, std::forward<Param>(value)...);
				}
			}
			void InvokeGroupPassConditionReverse(const uint index, Type* type, Param&&... value)
			{
				const int count = (int)innerGroup[index].size();
				for (uint i = 0; i < count; ++i)
				{
					int innerIndex = count - i - 1;
					if (Parent::Has(innerIndex))
						Parent::InvokePassCondition(innerGroup[index][innerIndex], type, std::forward<Param>(value)...);
				}
			}
		public:
			void RegisterG(const uint groupIndex, const uint funcIndex)
			{
				innerGroup[(uint)groupIndex].push_back(funcIndex);
			}
			void DeRegisterG(const uint index)
			{
				innerGroup[(uint)index].clear();
			}
			void DeRegisterG(const uint index, const uint innerIndex)
			{
				innerGroup[(uint)index].erase(innerGroup[(uint)index].begin() + innerIndex);
			}
		};
	}
}