#pragma once
#include<vector>
#include<unordered_map>
#include"../JDataType.h" 

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
			int Erase(Type* obj, EqualPtr equal)noexcept
			{
				if (obj == nullptr)
					return -1;

				const uint objectCount = (uint)objectVec.size();
				for (uint i = 0; i < objectCount; ++i)
				{
					if (equal(obj, objectVec[i]))
					{  
						objectVec.erase(objectVec.begin() + i);
						return i;
					}
				}
				return -1;
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
			Type* Add(Type* obj, const IdentifierType iden)noexcept
			{ 
				if (obj == nullptr)
					return nullptr;

				if (objectMap.find(iden) == objectMap.end())
				{
					objectMap.emplace(iden, obj);
					return obj;
				}
				else
					return nullptr;
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