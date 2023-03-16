#pragma once   
#include"JTypeInfoInitializer.h" 
#include"JTypeInfoOption.h"
#include"JReflectionInfo.h" 
#include"../JDataType.h"
#include<unordered_map>  
#include<vector>
#include<assert.h>  
#include<memory>

namespace JinEngine
{ 
	namespace Core
	{
		class JIdentifier;
		class JPropertyInfo;
		class JMethodInfo;  
		template<typename T> class JOwnerPtr;
		template<typename T> class JUserPtr;

		using JTypeInstance = JIdentifier;
		using IdentifierType = size_t;
		using TypeInstanceMap = std::unordered_map<IdentifierType, JOwnerPtr<JTypeInstance>>;
		using TypeInstanceVector = std::vector<JTypeInstance*>;
		using PropertyVec = std::vector<JPropertyInfo*>;
		using PropertyMap = std::unordered_map<std::string, JPropertyInfo*>;
		using MethodVec = std::vector<JMethodInfo*>;
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
			PropertyVec propertyInfoVec;
			PropertyMap propertyInfoMap;
			MethodVec methodInfoVec;
			MethodMap methodInfoMap;
		};
 
		class JTypeInfoOption;
		class JTypeInfo 
		{
		private: 
			friend class JIdentifier;
			friend class JReflectionInfoImpl;
			template<typename Type> friend class JTypeInfoRegister;
			template<typename Type, typename Field, typename Pointer, Pointer ptr> friend class JPropertyInfoRegister;
			template<typename Type, typename Field, typename Pointer, Pointer ptr> friend class JPropertyExInfoRegister;
			template<typename Type, typename Pointer, Pointer ptr> friend class JMethodInfoRegister;
		private:
			//name is class Name except namespace and class
			const std::string name;
			//typeid(T).name()
			const std::string fullName;
			const size_t hashCode;
			JTypeInfo* parent;
			JTypeInfoOption option;
			std::unique_ptr<JTypeInstanceData> instanceData;
			std::unique_ptr<JTypeMemberData> memberData;
		public:
			//just class name
			std::string Name()const noexcept;
			//except prefix J 
			std::string NameWithOutPrefix()const noexcept;
			//typeid name
			std::string FullName()const noexcept; 
		public:
			const PropertyVec GetPropertyVec()const noexcept;
			const MethodVec GetMethodVec()const noexcept;
			JTypeInfo* GetParent()const noexcept;
			JPropertyInfo* GetProperty(const std::string& name)const noexcept;
			JMethodInfo* GetMethod(const std::string& name)const noexcept;
			JTypeInfoOption* GetOption()noexcept;
			uint GetInstanceCount()const noexcept;
		public:
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
			JOwnerPtr<JTypeInstance> ReleaseInstance(IdentifierType iden)noexcept;
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
				//option = std::make_unique<JTypeInfoOption>();
			} 
		};

		struct JTypeInstanceSearchHint
		{
		public:
			const std::string typeName;
			const size_t guid;
			const bool isValid;
		public:
			JTypeInstanceSearchHint();
			JTypeInstanceSearchHint(const JTypeInfo& info, const size_t guid);
			JTypeInstanceSearchHint(Core::JUserPtr<JIdentifier> iden); 
		};
	}
}