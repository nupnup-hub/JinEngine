#pragma once   
#include"JTypeInfoInitializer.h"
#include"JReflectionInfo.h" 
#include<unordered_map>  
#include<vector>
#include<assert.h>  
#include<memory>

namespace JinEngine
{
	class JObject; 
	namespace Core
	{
		class JPropertyInfo;
		class JMethodInfo;

		using JTypeInstance = JObject;
		using TypeInstanceMap = std::unordered_map<size_t, JTypeInstance*>;
		using TypeInstanceVector = std::vector<JTypeInstance*>;
		using PropertyMap = std::unordered_map<std::string, JPropertyInfo*>;
		using MethodMap = std::unordered_map<std::string, JMethodInfo*>;

		struct JTypeInstanceData
		{
		public:
			TypeInstanceVector classInstanceVec;
			TypeInstanceMap classInstanceMap;
		};

		struct JTypeMemberData
		{
		public:
			PropertyMap propertyInfo;
			MethodMap methodInfo;
		};

		class JTypeInfo
		{
		private: 
			friend class JReflectionImpl;
			template<typename Type, typename Field, typename Pointer, Pointer ptr> friend class JPropertyRegister;
			template<typename Type, typename Pointer, Pointer ptr, typename Func> friend class JMethodRegister;
		private:
			//name is class Name except namespace and class
			const std::string name;
			//typeid(T).name()
			const std::string fullName;
			const size_t hashCode;
			JTypeInfo* parent;
			std::unique_ptr<JTypeInstanceData> instanceData;
			std::unique_ptr<JTypeMemberData> memberData;
		public:
			template<typename Type>
			JTypeInfo(const JTypeInfoInitializer<Type>& initializer)
				:name(initializer.name),
				fullName(initializer.fullName),
				hashCode(initializer.hashCode),
				parent(initializer.parent)
			{ 
				JReflectionInfo::Instance().AddType(this);
			}  
			std::string Name()const noexcept;
			std::string FullName()const noexcept;
			const PropertyMap* GetPropertyMap()const noexcept;
			const MethodMap* GetMethodMap()const noexcept;
			bool IsA(const JTypeInfo& tar)const noexcept;
			bool IsChildOf(const JTypeInfo& parentCandidate)const noexcept;

			template<typename Type>
			bool AddInstance(Type* ptr)
			{
				if (std::is_null_pointer_v<Type> || std::is_void_v<Type>)
					return false;
				 
				if (instanceData == nullptr)
					instanceData = std::make_unique<JTypeInstanceData>();

				if (instanceData->classInstanceMap.find(ptr->guid) == instanceData->classInstanceMap.end() && IsA<Type>())
				{
					instanceData->classInstanceVec.emplace_back(ptr);
					instanceData->classInstanceMap.emplace(ptr->guid, ptr);
					return true;
				}
				else
					return false;
			}
			template<typename T>
			bool IsA()
			{
				return IsA(T::StaticTypeInfo());
			}
			template<typename T>
			bool IsChildOf()
			{
				return IsChildOf(T::StaticTypeInfo());
			}		
		private:
			bool AddPropertyInfo(JPropertyInfo* newProperty);
			bool AddMethodInfo(JMethodInfo* newMethod);
		};
	}
}