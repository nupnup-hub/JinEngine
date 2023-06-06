#pragma once   
#include"JTypeInfoInitializer.h" 
#include"JTypeInfoGuiOption.h" 
#include"JTypeAllocationCreator.h"
#include"JReflectionInfo.h"  
#include"JReflectionInfoPrivate.h"  
#include"../JDataType.h"
#include"../JEngineInfo.h"
#include"../Pointer/JOwnerPtr.h"
#include"../Lazy/JLazyDestruction.h"
#include<unordered_map>  
#include<vector>
#include<assert.h>  
#include<memory>

namespace JinEngine
{
	namespace Core
	{
		class JTypeBase; 
		class JTypeBasePrivate;
		class JTypeImplBase;
		class JPropertyInfo;
		class JMethodInfo;
		//template<typename T> class JOwnerPtr;
		//template<typename T> class JUserPtr;

		using JTypeInstance = JTypeBase;
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
		 
		class JReflectionInfo;
		class JTypeInfo final
		{
		private:
			friend class JTypeBasePrivate;
			friend class JReflectionInfo;
		private:
			template<typename Type> friend class JTypeInfoRegister;
			template<typename Type, typename Field, typename Pointer, Pointer ptr> friend class JPropertyInfoRegister;
			template<typename Type, typename Field, typename Pointer, Pointer ptr> friend class JPropertyExInfoRegister;
			template<typename Type, typename Pointer, Pointer ptr> friend class JMethodInfoRegister;
		private:
			struct AllocationInitInfo
			{
			public:
				std::unique_ptr<JAllocationDesc> option;
				std::unique_ptr<JTypeAllocationCreatorInterface> creator;
			};
			struct ExtraFunctionInitInfo
			{
			public:
				std::unique_ptr<AllocationInitInfo> allocInitInfo;	//has default value
				std::unique_ptr<JLazyDestructionInfo> lazyDestructionInfo;	//have to call SetDestructionInfo function
			public:
				bool canUseAlloc = false;
				bool canUseLazy = false;
			};
			struct InterfaceTypeInfo
			{
			public: 
				using ConvertInterfacePtr = JTypeBase * (*)(JTypeImplBase*); 
			public:
				Core::JTypeInfo& interfaceType;
				const ConvertInterfacePtr convertPtr;
			public: 
				InterfaceTypeInfo(Core::JTypeInfo& interfaceType, const ConvertInterfacePtr convertPtr);
			};
			struct ImplTypeInfo
			{
			public:
				using ConvertImplBasePtr = JTypeImplBase*(*)(JTypeBase*); 
			public:
				Core::JTypeInfo& implType;
				const ConvertImplBasePtr convertPtr; 
			public:
				ImplTypeInfo(Core::JTypeInfo& implType, const ConvertImplBasePtr convertPtr); 
			};
		private: 
			using CallOnecePtr = void(*)();
		public:
			//멤버 함수가 존재해도
			//basic template 인수가 specialize template인수와 다르면 특수화 되지않음
			//basic == template<typename T, typename = int> 일시 call<A> => call<A, int>가된다.
			//그러므로 std::void_t에 타입인 void로 디폴트 설정
			template<typename T, typename = void>
			struct HasEngineTypeData : std::false_type
			{};
			template<typename T>
			struct HasEngineTypeData<T, std::void_t<decltype(&T::RegisterTypeData)>> : std::true_type
			{};
			template<typename T, typename = void>
			struct IsEngineAllocatorUser : std::false_type
			{};
			template<typename T>
			struct IsEngineAllocatorUser<T, std::void_t<decltype(&T::InitAllocatorInfo)>> : std::true_type
			{};
			template<typename T, typename = void>
			struct IsLazyDestructionUser : std::false_type
			{};
			template<typename T>
			struct IsLazyDestructionUser<T, std::void_t<decltype(&T::InitLazyDestructionInfo)>> : std::true_type
			{};

			template<typename Type>
			class CallOnece
			{
			public:
				static void Execute()
				{				 
					if constexpr (HasEngineTypeData<Type>::value)
					{ 
						static bool isValid = true;
						if(isValid)
							Type::RegisterTypeData();
						isValid = false;
					}			
					if constexpr (IsEngineAllocatorUser<Type>::value)
					{
						static bool isValid = true;
						if (isValid)
							Type::InitAllocatorInfo();
						isValid = false;
					}
					if constexpr (IsLazyDestructionUser<Type>::value)
					{
						static bool isValid = true;
						if (isValid)
							Type::InitLazyDestructionInfo();
						isValid = false;
					}
				}
				static constexpr bool IsDefinedInitAllocatorInfo()
				{
					if constexpr (IsEngineAllocatorUser<Type>::value)
						return true;
					else
						return false;
				}
				static constexpr bool IsDefinedInitLazyDestructionInfo()
				{
					if constexpr (IsLazyDestructionUser<Type>::value)
						return true;
					else
						return false;
				}
			};
		private:
			//name is class Name except namespace and class
			const std::string name;
			//typeid(T).name()
			const std::string fullName;
			const size_t hashCode;
			const size_t dataSize;
			JTypeInfo* parent; 
			JTypeInfoGuiOption option;
			std::unique_ptr<JTypeInstanceData> instanceData;
			std::unique_ptr<JTypeMemberData> memberData;
		private:
			CallOnecePtr callOncePtr = nullptr;
		private:
			std::unique_ptr<ExtraFunctionInitInfo> extraInitInfo;		//CallOnce 이후 nullptr
		private:
			std::unique_ptr<JAllocationInterface> allocationInterface; 
			std::unique_ptr<JLazyDestruction> lazyDestruction;
			std::unique_ptr<InterfaceTypeInfo> interfaceTypeInfo;
			std::unique_ptr<ImplTypeInfo> implTypeInfo;
		private:
			const bool isAbstractType;
			bool isLeafType = true; 
		public:
			//just class name	for display
			std::string Name()const noexcept;
			//except prefix J 
			std::string NameWithOutModifier()const noexcept;
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
			int GetInstanceIndex(IdentifierType iden)const noexcept;
		public:
			JTypeInstance* GetInstanceRawPtr(IdentifierType iden)const noexcept;
			JUserPtr<JTypeInstance> GetInstanceUserPtr(IdentifierType iden)const noexcept; 
			JWeakPtr<JTypeInstance> GetInstanceWeakPtr(IdentifierType iden)const noexcept;
			TypeInstanceVector GetInstanceRawPtrVec()const noexcept;
		public:
			template<typename T>
			T* GetInstanceRawPtr(IdentifierType iden)const noexcept
			{
				if (instanceData == nullptr)
					return nullptr;

				if (IsChildOf(T::StaticTypeInfo()))
				{
					auto data = instanceData->classInstanceMap.find(iden);
					return data != instanceData->classInstanceMap.end() ? static_cast<T*>(data->second.Get()) : nullptr;
				}
				else
					return nullptr;
			}
			template<typename T>
			JUserPtr<T> GetInstanceUserPtr(IdentifierType iden)const noexcept
			{ 
				if (instanceData == nullptr)
					return JUserPtr<T>{};

				if (IsChildOf(T::StaticTypeInfo()))
				{
					auto data = instanceData->classInstanceMap.find(iden);
					return data != instanceData->classInstanceMap.end() ? JUserPtr<T>::CreateChild(data->second) : JUserPtr<T>{};
				}
				else
					return JUserPtr<T>{};
			} 
			template<typename T>
			JWeakPtr<T> GetInstanceWeakPtr(IdentifierType iden)const noexcept
			{
				if (instanceData == nullptr)
					return JWeakPtr<T>{};

				if (IsChildOf(T::StaticTypeInfo()))
				{
					auto data = instanceData->classInstanceMap.find(iden);
					return data != instanceData->classInstanceMap.end() ? JWeakPtr<T>::CreateChild(data->second) : JWeakPtr<T>{};
				}
				else
					return JWeakPtr<T>{};
			}
		public:
			JAllocationInterface* GetAllocationInterface()const noexcept;
			JAllocationInfo GetAllocInfo()const noexcept;
		public:
			JTypeInfo* GetInterfaceTypeInfo()const noexcept;	//for impl class
			JTypeInfo* GetImplTypeInfo()const noexcept;			//for has impl class
		public:
			//is valid until CallOnce 
			bool SetAllocationCreator(std::unique_ptr <JTypeAllocationCreatorInterface>&& newCreator)noexcept;
			bool SetAllocationOption(std::unique_ptr<JAllocationDesc>&& newOption)noexcept;
			bool SetDestructionInfo(std::unique_ptr<JLazyDestructionInfo>&& newDesInfo)noexcept;
		public:
			bool IsAbstractType()const noexcept;
			bool IsLeafType()const noexcept;
			bool IsA(const JTypeInfo& tar)const noexcept;
			bool IsChildOf(const JTypeInfo& parentCandidate)const noexcept; 
			bool CanUseLazyDestruction()const noexcept;
			bool HasInterfaceTypeInfo()const noexcept;
			bool HasImplTypeInfo()const noexcept;
		public:
			template<typename T>
			bool IsA()const noexcept
			{
				return IsA(T::StaticTypeInfo());
			}
			template<typename T>
			bool IsChildOf()const noexcept
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
			JTypeBase* ConvertInterfaceBase(JTypeImplBase* implBase)const noexcept;
			JTypeImplBase* ConvertImplBase(JTypeBase* tBase)const noexcept;
		public:
			void TryLazyDestruction(JTypeBase* tBase)noexcept;
			void TryCancelLazyDestruction(JTypeBase* tBase)noexcept;
		private:
			void UpdateLazyDestruction(const float timeOffset)noexcept;
		private:
			bool AddInstance(IdentifierType iden, JOwnerPtr<JTypeInstance> ptr)noexcept;
			bool RemoveInstance(IdentifierType iden)noexcept;
			JOwnerPtr<JTypeInstance> ReleaseInstance(IdentifierType iden)noexcept;
		private:
			bool AddPropertyInfo(JPropertyInfo* newProperty);
			bool AddMethodInfo(JMethodInfo* newMethod);
		private:
			void ExecuteTypeCallOnece();
		private:
			//if allocation option is nullptr
			//set default allocation option
			void RegisterEngineDefaultAllocationOption();
			void RegisterAllocation();
			void DeRegisterAllocation();  
		private:
			void RegisterLazyDestructionInfo();
		private:
			void RegisterInterfaceTypeInfo(std::unique_ptr<InterfaceTypeInfo>&& interfaceTypeInfo);
			void RegisterImplTypeInfo(std::unique_ptr<ImplTypeInfo>&& implTypeInfo);
		private:
			void EndRegister();
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
				  
				JReflectionInfoPrivate::TypeInterface::AddType(this);
				extraInitInfo = std::make_unique<ExtraFunctionInitInfo>();
				if constexpr (CallOnece<Type>::IsDefinedInitAllocatorInfo())
				{
					extraInitInfo->canUseAlloc = true;
					extraInitInfo->allocInitInfo = std::make_unique<AllocationInitInfo>();
				}
				if constexpr (CallOnece<Type>::IsDefinedInitLazyDestructionInfo())
					extraInitInfo->canUseLazy = true;

				callOncePtr = &CallOnece<Type>::Execute;
				if (parent != nullptr)
					parent->isLeafType = false;
			}
			~JTypeInfo() = default;
		};

		struct JTypeInstanceSearchHint
		{
		public: 
			const size_t typeGuid;
			const size_t objectGuid;
			const bool isValid;
		public:
			const bool hasImplType = false;
		public:
			JTypeInstanceSearchHint();
			JTypeInstanceSearchHint(const JTypeInfo& info, const size_t guid);
			JTypeInstanceSearchHint(JUserPtr<JTypeBase> iden);
		};
	}
}