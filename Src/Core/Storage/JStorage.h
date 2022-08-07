#pragma once
#include<vector>
#include<unordered_map>
#include"../JDataType.h" 
#include"../Func/Callable/JCallable.h"

namespace JinEngine
{
	namespace Core
	{ 
		template<typename Type>
		class JVectorStorage
		{
			using ObjectVector = std::vector<Type*>;
			using ObjectVectorCIter = typename std::vector<Type*>::const_iterator;
			using EqualPtr = bool(*)(Type*, Type*);
		private:
			ObjectVector objectVec;
		public: 
			virtual ~JVectorStorage() {}
			Type* operator[](const uint index)
			{
				return objectVec[index];
			}
			void Resize(const uint count)
			{
				objectVec.resize(count);
			}
			uint Count()const noexcept
			{
				return (uint)objectVec.size();
			}
			Type* Get(const uint index)
			{
				return objectVec[index];
			}
			int GetIndex(Type* obj, EqualPtr equal)
			{
				const uint objectCount = (uint)objectVec.size();
				for (uint i = 0; i < objectCount; ++i)
				{
					if (equal(obj, objectVec[i]))
						return i;
				}
				return -1;
			}
			ObjectVectorCIter GetCBegin()
			{
				return objectVec.cbegin();
			}
			Type* Add(Type* obj)noexcept
			{
				if (obj == nullptr)
					return nullptr;

				objectVec.push_back(obj);
				return obj;
			}
			bool Erase(const int index)
			{
				if (Get(index) != nullptr)
				{
					objectVec.erase(objectVec.begin() + index);
					return true;
				}
				else
					return false;
			}
			int Erase(Type* obj, EqualPtr equal)noexcept
			{
				if (obj == nullptr)
					return -1;

				int index = GetIndex(obj, equal);
				if (index != -1)
				{
					objectVec.erase(objectVec.begin() + index);
					return index;
				}
				else
					return -1;
			}
			template<typename Ret, typename ...Param>
			int Erase(Type* obj, EqualPtr equal, JStaticCallable<Ret, Type&, Param...> afProccessCallable, Param&&... var)noexcept
			{
				if (obj == nullptr)
					return -1;

				int index = GetIndex(obj, equal);
				if (index != -1)
				{
					const uint objectCount = (uint)objectVec.size();
					for (uint i = index + 1; i < objectCount; ++i)
						afProccessCallable(nullptr, *objectVec[i], std::forward<Param>(var)...);
					objectVec.erase(objectVec.begin() + index);
					return index;
				}
				else
					return -1;
			}
			template<typename Ret, typename ...Param>
			void ApplyFunc(const uint index, JStaticCallable<Ret, Type&, Param...> callable, Param&&... var)
			{
				if (Get(index) != nullptr)
				{
					const uint objectCount = (uint)objectVec.size();
					for (uint i = index; i < objectCount; ++i)
						callable(nullptr, *objectVec[i], std::forward<Param>(var)...);
				}
			}
			template<typename Ret, typename ...Param>
			void ApplyFuncByIndex(const uint index, JStaticCallable<Ret, Type&, const uint&, Param...> callable, Param&&... var)
			{
				if (Get(index) != nullptr)
				{
					const uint objectCount = (uint)objectVec.size();
					for (uint i = index; i < objectCount; ++i)
						callable(nullptr, *objectVec[i], i, std::forward<Param>(var)...);
				}
			}
			void Clear()
			{
				objectVec.clear();
			}
		};

		template<typename Type, typename IdentifierType>
		class JMapStorage
		{
			using ObjectMap = std::unordered_map<IdentifierType, Type*>;
			using EqualPtr = bool(*)(Type*, Type*);
		private:
			ObjectMap objectMap;
		public: 
			virtual ~JMapStorage(){}
			uint Count()const noexcept
			{
				return objectMap.size();
			}
			Type* Get(const IdentifierType iden)
			{
				auto data = objectMap.find(iden);
				return data != objectMap.end() ? data->second : nullptr;
			}
			bool Has(const IdentifierType iden)
			{
				return  objectMap.find(iden) != objectMap.end();
			}
			Type* Add(Type* obj, const IdentifierType iden)noexcept
			{ 
				if (obj == nullptr)
					return nullptr;

				objectMap.emplace(iden, obj);
				return obj;
			}
			bool Erase(Type* obj, EqualPtr equal)noexcept
			{
				if (obj == nullptr)
					return false;

				for (auto& data : objectMap)
				{
					if (equal(data, obj))
					{
						objectMap.erase(data);
						return true;
					}
				}
				return false;
			}
			bool Erase(const IdentifierType iden)noexcept
			{
				if (objectMap.find(iden) != objectMap.end())
				{
					objectMap.erase(iden);
					return true;
				}
				else
					return false;
			}
			void Clear()
			{
				objectMap.clear();
			}
		};


	}
}