#pragma once   
#include"JTypeInfoInitializer.h" 
#include"JTypeInfoGuiOption.h" 
#include"JReflectionInfo.h" 
#include"JTypeAllocationOption.h"
#include"../JDataType.h"
#include"../Pointer/JOwnerPtr.h"
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
		//template<typename T> class JOwnerPtr;
		//template<typename T> class JUserPtr;

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

		struct JTypeAllocationOption;
		class JAllocationInterface;

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
			using CallOnecePtr = void(*)();
		public:
			//멤버 함수가 존재해도
			//basic template 인수가 specialize template인수와 다르면 특수화 되지않음
			//basic == template<typename T, typename = int> 일시 call<A> => call<A, int>가된다.
			//그러므로 std::void_t에 타입인 void로 디폴트 설정
			template<typename T, typename = void>
			struct HasEngineDefinedRegister : std::false_type
			{};
			template<typename T>
			struct HasEngineDefinedRegister<T, std::void_t<decltype(&T::RegisterCallOnce)>> : std::true_type
			{};
			template<typename Type>
			class CallOnece
			{
			public:
				static void Execute()
				{				 
					if constexpr (HasEngineDefinedRegister<Type>::value)
					{
						static bool isValid = true;
						if(isValid)
							Type::RegisterCallOnce();
						isValid = false;
					}
				}
			};
		private:
			//name is class Name except namespace and class
			const std::string name;
			//typeid(T).name()
			const std::string fullName;
			const size_t hashCode;
			const size_t dataSize;
			const bool isAbstractType;
			bool isLeafType = true;
			JTypeInfo* parent;
			JTypeInfoGuiOption option;
			std::unique_ptr<JTypeInstanceData> instanceData;
			std::unique_ptr<JTypeMemberData> memberData;
		private:
			CallOnecePtr callOncePtr = nullptr;
		private:
			std::unique_ptr<JTypeAllocationOption> allocationOption;
			std::unique_ptr<JTypeAllocationCreatorInterface> allocationCreator;
			std::unique_ptr<JAllocationInterface> allocationInterface;
			bool isRegisteredAllocation = false;
		public:
			//just class name	for display
			std::string Name()const noexcept;
			//except prefix J 
			std::string NameWithOutPrefix()const noexcept;
			//typeid name	for guid
			std::string FullName()const noexcept;
			//created by fullname 
			size_t TypeGuid()const noexcept;
		public:
			const PropertyVec GetPropertyVec()const noexcept;
			const MethodVec GetMethodVec()const noexcept;
			JTypeInfo* GetParent()const noexcept;
			JPropertyInfo* GetProperty(const std::string& name)const noexcept;
			JMethodInfo* GetMethod(const std::string& name)const noexcept;
			JTypeInfoGuiOption* GetOption()noexcept;
			uint GetInstanceCount()const noexcept;
		public:
			JTypeInstance* GetInstanceRawPtr(IdentifierType iden)noexcept;
			JUserPtr<JTypeInstance> GetInstanceUserPtr(IdentifierType iden)noexcept;
		public:
			JAllocationInterface* GetAllocationInterface()noexcept;
		public:
			//is valid until create allocation instance
			//so it has to called until app run
			bool SetAllocationCreator(std::unique_ptr < JTypeAllocationCreatorInterface>&& newCreator)noexcept;
			bool SetAllocationOption(std::unique_ptr<JTypeAllocationOption>&& newOption)noexcept;
		public:
			bool IsAbstractType()const noexcept;
			bool IsLeafType()const noexcept;
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
		private:
			bool AddInstance(IdentifierType iden, JOwnerPtr<JTypeInstance> ptr)noexcept;
			bool RemoveInstance(IdentifierType iden)noexcept;
			JOwnerPtr<JTypeInstance> ReleaseInstance(IdentifierType iden)noexcept;
		private:
			bool AddPropertyInfo(JPropertyInfo* newProperty);
			bool AddMethodInfo(JMethodInfo* newMethod);
		private:
			void ExecuteTypeCallOnece();
			//if allocation option is nullptr
			//set default allocation option
			void RegisterEngineDefaultAllocationOption();
			void RegisterAllocation();
			void DeRegisterAllocation();
		private:
			template<typename Type>
			JTypeInfo(const JTypeInfoInitializer<Type>& initializer)
				:name(initializer.name),
				fullName(initializer.fullName),
				hashCode(initializer.hashCode),
				dataSize(sizeof(Type)),
				parent(initializer.parent),
				isAbstractType(std::is_abstract_v<Type>)
			{
				if (std::is_base_of_v<JTypeInstance, Type>)
					instanceData = std::make_unique<JTypeInstanceData>();
				 
				JReflectionInfo::Instance().AddType(this);
				callOncePtr = &CallOnece<Type>::Execute;
				allocationCreator = std::make_unique<JTypeAllocationCreator<Type>>();
				if (parent != nullptr)
					parent->isLeafType = false;
			}
		};

		struct JTypeInstanceSearchHint
		{
		public: 
			const size_t typeGuid;
			const size_t objectGuid;
			const bool isValid;
		public:
			JTypeInstanceSearchHint();
			JTypeInstanceSearchHint(const JTypeInfo& info, const size_t guid);
			JTypeInstanceSearchHint(Core::JUserPtr<JIdentifier> iden);
		};
	}
}