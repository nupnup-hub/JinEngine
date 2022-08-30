#pragma once   
#include"JTypeInfoInitializer.h"
#include"JReflectionInfo.h" 
#include"../JDataType.h"
#include"../Pointer/JOwnerPtr.h"
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
		using IdentifierType = size_t;
		using TypeInstanceMap = std::unordered_map<IdentifierType, JOwnerPtr<JTypeInstance>>;
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
			friend class JObject;
			friend class JReflectionImpl;
			template<typename Type> friend class JTypeInfoRegister;
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
			std::string Name()const noexcept;
			std::string FullName()const noexcept;
			const PropertyMap* GetPropertyMap()const noexcept;
			const MethodMap* GetMethodMap()const noexcept;
			bool IsA(const JTypeInfo& tar)const noexcept;
			bool IsChildOf(const JTypeInfo& parentCandidate)const noexcept;
		public:
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
			template<typename ...Param>
			void InvokeInstanceFunc(void(JTypeInstance::* ptr)(Param...), Param... var)
			{
				if (instanceData != nullptr)
				{
					const uint instanceCount = (uint)instanceData->classInstanceVec.size();
					for (uint i = 0; i < instanceCount; ++i)
						(instanceData->classInstanceVec[i].Get()->*ptr)(std::forward<Param>(var)...);
				}
			}
		public:
			JTypeInstance* GetInstanceRawPtr(IdentifierType iden)noexcept;
			JUserPtr<JTypeInstance> GetInstanceUserPtr(IdentifierType iden)noexcept;
		private:
			bool AddInstance(IdentifierType iden, JOwnerPtr<JTypeInstance> ptr)noexcept;
			bool RemoveInstance(IdentifierType iden)noexcept;
		private:
			bool AddPropertyInfo(JPropertyInfo* newProperty);
			bool AddMethodInfo(JMethodInfo* newMethod);
		private:
			template<typename Type>
			JTypeInfo(const JTypeInfoInitializer<Type>& initializer)
				:name(initializer.name),
				fullName(initializer.fullName),
				hashCode(initializer.hashCode),
				parent(initializer.parent)
			{
				if(std::is_base_of_v<JTypeInstance, Type>)
					instanceData = std::make_unique<JTypeInstanceData>();

				JReflectionInfo::Instance().AddType(this);
			} 
		};
	}
}