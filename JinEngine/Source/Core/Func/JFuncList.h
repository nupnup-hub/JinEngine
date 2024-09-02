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
#include"Callable/JCallable.h" 

namespace JinEngine
{
	namespace Core
	{
		//enum value has to separate one by one
		template<uint count, typename Type, typename ...Param>
		class JFuncList
		{
		public:
			enum class CONDITION_MASK
			{
				PASS_ALL,
				PASS_LOCAL,
				PASS_NONE,
			};
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
			std::unique_ptr<JCondCallable> globalCond = nullptr;
		public:
			JFuncList() = default;
		public:
			void Invoke(const uint index, Type* type, Param&&... value)
			{
				if (globalCond != nullptr && !(*globalCond)(type))
					return;

				if (list[index].cond != nullptr && !(*list[index].cond)(type))
					return;

				(*list[index].func)(type, type, std::forward<Param>(value)...);
			}
			void InvokePassLocalCondition(const uint index, Type* type, Param&&... value)
			{
				if (globalCond != nullptr && !(*globalCond)(type))
					return;

				(*list[index].func)(type, type, std::forward<Param>(value)...);
			}
			void InvokePassAllCondition(const uint index, Type* type, Param&&... value)
			{ 
				(*list[index].func)(type, type, std::forward<Param>(value)...);
			}
			void InvokeAll(Type* type, const CONDITION_MASK condMask, Param&&... value)
			{ 
				for (uint i = 0; i < count; ++i)
				{
					if (Has(i))
					{
						if (condMask == CONDITION_MASK::PASS_NONE)
							Invoke(i, type, std::forward<Param>(value)...);
						else if (condMask == CONDITION_MASK::PASS_LOCAL)
							InvokePassLocalCondition(i, type, std::forward<Param>(value)...);
						else
							InvokePassAllCondition(i, type, std::forward<Param>(value)...);
					}
				}
			}
			void InvokeAllReverse(Type* type, const CONDITION_MASK condMask, Param&&... value)
			{ 
				for (uint i = 0; i < count; ++i)
				{
					int index = count - i - 1;
					if (Has(index))
					{
						if (condMask == CONDITION_MASK::PASS_NONE)
							Invoke(index, type, std::forward<Param>(value)...);
						else if (condMask == CONDITION_MASK::PASS_LOCAL)
							InvokePassLocalCondition(index, type, std::forward<Param>(value)...);
						else
							InvokePassAllCondition(index, type, std::forward<Param>(value)...);
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
		public:
			void RegisterGlobalCond(std::unique_ptr<JCondCallable>&& newGlobalCond)
			{
				globalCond = std::move(newGlobalCond);
			}
			void DeRegisterGlobalCond()
			{
				globalCond = nullptr;
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
						Parent::InvokePassLocalCondition(innerGroup[index][i], type, std::forward<Param>(value)...);
				}
			}
			void InvokeGroupPassConditionReverse(const uint index, Type* type, Param&&... value)
			{
				const int count = (int)innerGroup[index].size();
				for (uint i = 0; i < count; ++i)
				{
					int innerIndex = count - i - 1;
					if (Parent::Has(innerIndex))
						Parent::InvokePassLocalCondition(innerGroup[index][innerIndex], type, std::forward<Param>(value)...);
				}
			}
			void InvokeGroupPassAllCondition(const uint index, Type* type, Param&&... value)
			{
				const uint count = (uint)innerGroup[index].size();
				for (uint i = 0; i < count; ++i)
				{
					if (Parent::Has(i))
						Parent::InvokePassAllCondition(innerGroup[index][i], type, std::forward<Param>(value)...);
				}
			}
			void InvokeGroupPassAllConditionReverse(const uint index, Type* type, Param&&... value)
			{
				const int count = (int)innerGroup[index].size();
				for (uint i = 0; i < count; ++i)
				{
					int innerIndex = count - i - 1;
					if (Parent::Has(innerIndex))
						Parent::InvokePassAllCondition(innerGroup[index][innerIndex], type, std::forward<Param>(value)...);
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