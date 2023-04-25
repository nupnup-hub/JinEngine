#pragma once    
#include"../Pointer/JOwnerPtr.h"
#include"../../Utility/JMacroUtility.h" 
#include<type_traits> 

namespace JinEngine
{
	namespace Core
	{
		namespace
		{ 
			template<typename T, typename U = void>
			struct Depth
			{
			public:
				enum { value = 0 };
			};

			template<typename T>
			struct Depth<T, std::void_t<typename T::ThisType>>
			{
			public:
				enum { value = 1 + Depth<T::ParentType>::value };
			};

			template<typename T, typename U = void>
			struct ParentClass
			{
			public:
				using Type = void;
			};

			template<typename T>
			struct ParentClass<T, std::void_t<typename T::ThisType>>
			{
			public: 
				using Type = typename T::ThisType;
			};

#define IDEN_IMPL_BASE_CLASS JinEngine::Core::JIdentifierImplBase
#define IDEN_BASE_CLASS JinEngine::Core::JIdentifier

#define CONVERT_IMPL_BASE_PTR(baseImplName, interfaceName, baseInterfaceName) [](baseInterfaceName* ptr) ->baseImplName* { return static_cast<interfaceName*>(ptr)->impl.get();}


		}

#pragma region Register 

		//Has order dependecy
		//type register 호출하기전에 선언되어있어야함
#define REGISTER_CLASS_DEFAULT_ALLOCATOR private: static void InitAllocatorInfo(){}					 


					//Has order dependecy
#define REGISTER_CLASS_DEFINITION_TYPE(typeName, ...)														\
																										\
		public:																							\
			using TypeDepth = JinEngine::Core::Depth<typeName>;											\
			using ParentType = typename JinEngine::Core::ParentClass<typeName>::Type;					\
			using ThisType = typeName;																	\
																										\
		protected:																						\
			using JTypeInfo = JinEngine::Core::JTypeInfo;										\
			using JTypeInfoInitializer = JinEngine::Core::JTypeInfoInitializer<typeName>;		\
			using JTypeInfoCallOnece = JinEngine::Core::JTypeInfo::CallOnece<typeName>;			\
			using JTypeInfoRegister = JinEngine::Core::JTypeInfoRegister<typeName>;				\
			using JReflectionInfo = JinEngine::Core::JReflectionInfo;							\
			using JPtrUtil = JinEngine::Core::JPtrUtil;											\
																								\
		private:																				\
			friend class JTypeInfoInitializer;													\
			friend class JTypeInfoCallOnece;													\
			template<typename T> friend class JinEngine::Core::JOwnerPtr;						\
			friend class JPtrUtil;																\
																								\


#define REGISTER_IMPL_CLASS_INTERFACE_TYPE	using ThisInterfaceType = ThisType;
#define REGISTER_INNER_CLASS_OUTER_TYPE	using OuterType = ThisType;

#define REGISTER_CLASS_TYPE_INFO_CREATOR(typeName, ...)											\
		private:																				\
			inline static struct typeName##TypeInfoInstance										\
			{																					\
			public:																				\
				typeName##TypeInfoInstance(){ static JTypeInfoRegister typeRegister{#typeName};	}\
			}typeName##TypeInfoInstance;														\
																								\


#define REGISTER_CLASS_TYPE_INFO_CREATOR_IMPL(implName, ...)									\
		private:																				\
			inline static struct implName##TypeInfoInstance										\
			{																					\
			public:																				\
				implName##TypeInfoInstance()													\
				{																				\
					static JTypeInfoRegister typeRegister{ #implName,							\
						typeid(ThisInterfaceType).name(),										\
						ThisInterfaceType::StaticTypeInfo(),									\
						CONVERT_IMPL_BASE_PTR(IDEN_IMPL_BASE_CLASS, ThisInterfaceType, IDEN_BASE_CLASS) };\
				}																				\
			}implName##TypeInfoInstance;														\


#define REGISTER_CLASS_TYPE_FUNC(typeName, ...)													\
																								\
		public:																					\
			static JTypeInfo& StaticTypeInfo()noexcept											\
			{																					\
				static JTypeInfo& thisTypeInfo = *JReflectionInfo::Instance().GetTypeInfo(typeid(typeName).name());		\
				return thisTypeInfo;															\
			}																					\
			virtual JTypeInfo& GetTypeInfo()const noexcept {return typeInfo;}					\
			virtual int GetTypeDepth()const noexcept{return TypeDepth::value;}					\
			inline static std::string TypeName()noexcept {return #typeName;}					\
			inline static std::wstring TypeWName()noexcept {return L#typeName;}					\
		private:																				\
			inline static JTypeInfo& typeInfo = StaticTypeInfo();								\
																								\


#define REGISTER_CLASS_ALLOC_FUNC																\
																								\
		public:																					\
			static void* operator new(size_t size)												\
			{																					\
				if constexpr (std::is_base_of_v<JinEngine::Core::JIdentifier, ThisType>)		\
				{																				\
					auto allocInterface = typeInfo.GetAllocationInterface();					\
					if (allocInterface != nullptr)												\
						return allocInterface->Allocate(size);									\
					else                                                                        \
						return ::operator new(size);											\
				}																				\
				else                                                                            \
					return ::operator new(size);												\
			}																					\
			static void* operator new[](size_t size)											\
			{																					\
				if constexpr (std::is_base_of_v<JinEngine::Core::JIdentifier, ThisType>)		\
				{																				\
					auto allocInterface = typeInfo.GetAllocationInterface();					\
					if (allocInterface != nullptr)												\
						return allocInterface->Allocate(size);									\
					else                                                                        \
						return ::operator new(size);											\
				}																				\
				else                                                                            \
					return ::operator new(size);												\
			}																					\
			static void operator delete(void* p)												\
			{																					\
				if constexpr (std::is_base_of_v<JinEngine::Core::JIdentifier, ThisType>)		\
				{																				\
					auto allocInterface = typeInfo.GetAllocationInterface();					\
					if (allocInterface != nullptr)												\
						allocInterface->DeAllocate(p);											\
					else																		\
						::operator delete(p);													\
				}																				\
				else																			\
					::operator delete(p);														\
			}																					\
			static void operator delete[](void* p, size_t size)									\
			{																					\
				if constexpr (std::is_base_of_v<JinEngine::Core::JIdentifier, ThisType>)		\
				{																				\
					auto allocInterface = typeInfo.GetAllocationInterface();					\
					if (allocInterface != nullptr)												\
						allocInterface->DeAllocate(p, size);									\
					else																		\
						::operator delete(p);													\
				}																				\
				else																			\
					::operator delete(p);													    \
			}																					\
																								\



#define REGISTER_CLASS_IDENTIFIER_LINE(typeName, ...)												\
		REGISTER_CLASS_DEFINITION_TYPE(typeName, __VA_ARGS__)										\
		REGISTER_CLASS_DEFAULT_ALLOCATOR															\
		REGISTER_CLASS_TYPE_INFO_CREATOR(typeName, __VA_ARGS__)										\
		REGISTER_CLASS_TYPE_FUNC(typeName, __VA_ARGS__)												\
		REGISTER_CLASS_ALLOC_FUNC															\
																									
																								
#define REGISTER_CLASS_IDENTIFIER_LINE_IMPL(typeName, ...)										\
		REGISTER_IMPL_CLASS_INTERFACE_TYPE														\
		REGISTER_CLASS_DEFINITION_TYPE(typeName, __VA_ARGS__)									\
		REGISTER_CLASS_DEFAULT_ALLOCATOR														\
		REGISTER_CLASS_TYPE_INFO_CREATOR_IMPL(typeName, __VA_ARGS__)							\
		REGISTER_CLASS_TYPE_FUNC(typeName, __VA_ARGS__)											\
		REGISTER_CLASS_ALLOC_FUNC																\


#define REGISTER_CLASS_ONLY_USE_TYPEINFO(typeName, ...)													\
		REGISTER_CLASS_DEFINITION_TYPE(typeName, __VA_ARGS__)											\
		REGISTER_CLASS_TYPE_INFO_CREATOR(typeName, __VA_ARGS__)											\
		REGISTER_CLASS_TYPE_FUNC(typeName, __VA_ARGS__)													\


#define SEMICOLON ;

		//Parameter
		//REGISTER_PROPERTY(name, definedGetFunc, definedSetFunc, support editor gui type)

		namespace ReflectionData
		{

#define REGISTER_PROPERTY(propertyName, ...)														\
			template<typename Class, typename Field, typename Pointer, Pointer ptr>					\
			class JinEngine::Core::JPropertyInfoRegister;											\
																									\
			inline static struct propertyName##PropertyStruct										\
			{																						\
				public:																				\
					propertyName##PropertyStruct()													\
					{																				\
						static JinEngine::Core::JPropertyInfoRegister<ThisType,							\
						decltype(propertyName),														\
						decltype(&ThisType::propertyName),											\
						&ThisType::propertyName> jPropertyRegister{#propertyName, __VA_ARGS__};	\
					}																				\
			}propertyName##Property;																\
																									\

#define REGISTER_PROPERTY_EX(propertyName, getName, setName, ...)														\
			template<typename Class, typename Field, typename Pointer, Pointer ptr>					\
			class JinEngine::Core::JPropertyExInfoRegister;												\
																									\
			inline static struct propertyName##PropertyStruct										\
			{																						\
				public:																				\
					propertyName##PropertyStruct()													\
					{																				\
						static JinEngine::Core::JPropertyExInfoRegister<ThisType,							\
						decltype(propertyName),														\
						decltype(&ThisType::propertyName),											\
						&ThisType::propertyName> jPropertyRegister{#propertyName, &ThisType::getName, &ThisType::setName, __VA_ARGS__};					\
					}																				\
			}propertyName##Property;																\


#define REGISTER_METHOD(methodName, ...)																\
			template<typename Class, typename Pointer, Pointer ptr>										\
			class JinEngine::Core::JMethodInfoRegisterHelper;											\
																										\
			inline static struct methodName##MethodStruct												\
			{																							\
				public:																					\
					methodName##MethodStruct()															\
					{																					\
						static JinEngine::Core::JMethodInfoRegisterHelper<ThisType,						\
						decltype(&ThisType::methodName),												\
						&ThisType::methodName>															\
						jMethodInfoRegisterHelper{#methodName, #methodName, __VA_ARGS__};				\
					}																					\
			}methodName##Method;																		\


#define REGISTER_OVERLAOD_METHOD(methodName, ret, ...)													\
			template<typename Class, typename Pointer, Pointer ptr>										\
			class JinEngine::Core::JMethodInfoRegisterHelper;											\
																										\
			inline static struct J_MERGE_NAME(methodName, __VA_ARGS__)									\
			{																							\
				public:																					\
					J_MERGE_NAME(methodName, __VA_ARGS__)()												\
					{																					\
						using MethodType = ret(ThisType::*)(__VA_ARGS__);								\
						static JinEngine::Core::JMethodInfoRegisterHelper<ThisType,						\
						MethodType,																		\
						static_cast<MethodType>(&ThisType::methodName)>								    \
						jMethodInfoRegisterHelper{J_STRINGIZE(methodName), J_STRINGIZE(__VA_ARGS__)};	\
					}																					\
			} J_MERGE_NAME(methodName, __VA_ARGS__);													\
	

#define REGISTER_METHOD_READONLY_GUI_WIDGET(displayName, getName, ...)												\
			template<typename Class,  typename GetPointer, GetPointer getPtr>							\
			class JinEngine::Core::JMethodReadOnlyGuiWidgetRegister;											\
																										\
			inline static struct J_MERGE_NAME(displayName, getName)										\
			{																							\
				public:																					\
					J_MERGE_NAME(displayName, getName)()												\
					{																					\
						static JinEngine::Core::JMethodReadOnlyGuiWidgetRegister<ThisType,				\
						decltype(&ThisType::getName),													\
						&ThisType::getName>																\
						jMethodGuiRegisterHelper{J_STRINGIZE(displayName), J_STRINGIZE(getName), __VA_ARGS__};					\
					}																					\
			} J_MERGE_NAME(displayName, getName);														\


#define REGISTER_METHOD_GUI_WIDGET(displayName, getName, setName, ...)												\
			template<typename Class,  typename GetPointer, GetPointer getPtr>							\
			class JinEngine::Core::JMethodGuiWidgetRegister;											\
																										\
			inline static struct J_MERGE_NAME(J_MERGE_NAME(displayName, getName), setName)				\
			{																							\
				public:																					\
					J_MERGE_NAME(J_MERGE_NAME(displayName, getName),setName)()							\
					{																					\
						static JinEngine::Core::JMethodGuiWidgetRegister<ThisType,						\
						decltype(&ThisType::getName), 													\
						&ThisType::getName>																\
						jMethodGuiRegisterHelper{J_STRINGIZE(displayName),J_STRINGIZE(getName), J_STRINGIZE(setName), __VA_ARGS__};\
					}																					\
			} J_MERGE_NAME(J_MERGE_NAME(displayName, getName),setName);									\


#define REGISTER_PARENT_METHOD_GUI_WIDGET(parentType, displayName, getName, setName, ...)				\
			template<typename Class,  typename GetPointer, GetPointer getPtr>							\
			class JinEngine::Core::JMethodGuiWidgetRegister;											\
																										\
			inline static struct J_MERGE_NAME(J_MERGE_NAME(displayName, getName), setName)				\
			{																							\
				public:																					\
					J_MERGE_NAME(J_MERGE_NAME(displayName, getName), setName)()							\
					{																					\
						static JinEngine::Core::JMethodGuiWidgetRegister<parentType,						\
						decltype(&parentType::getName), 													\
						&parentType::getName>																\
						jMethodGuiRegisterHelper{J_STRINGIZE(displayName),J_STRINGIZE(getName), J_STRINGIZE(setName), __VA_ARGS__};\
					}																					\
			} J_MERGE_NAME(J_MERGE_NAME(displayName, getName),setName);									\


		}

		//enum class n {...., Count}
		//Added Count and Count name don't registeted
	#define REGISTER_ENUM_CLASS(enumName, dataType, ...)													\
			enum class enumName : dataType	{J_MAKE_ENUM_ELEMENT(__VA_ARGS__), COUNT};					\
																										\
			namespace ReflectionData																	\
			{																							\
				inline static struct J_MERGE_NAME(enumName, EnumStruct)										\
				{																							\
					public:																					\
						J_MERGE_NAME(enumName, EnumStruct)	()												\
						{																					\
							using JEnumInitializer = JinEngine::Core::JEnumInitializer;						\
							using JEnumInfo	= JinEngine::Core::JEnumInfo;									\
							using JEnumRegister	= JinEngine::Core::JEnumRegister<enumName>;					\
							static JEnumRegister jEnumRegister{J_STRINGIZE(enumName),						\
							J_STRINGIZE_ADD_COMMA(__VA_ARGS__), J_COUNT(__VA_ARGS__), true};				\
						}																					\
				}J_MERGE_NAME(enumName, EnumStruct);														\
			}																								\


#define REGISTER_ENUM(enumName, dataType, ...)															\
			enum enumName : dataType	{J_MAKE_ENUM_ELEMENT(__VA_ARGS__)};								\
																										\
			namespace ReflectionData																	\
			{																							\
				inline static struct J_MERGE_NAME(enumName, EnumStruct)										\
				{																							\
					public:																					\
						J_MERGE_NAME(enumName, EnumStruct)	()												\
						{																					\
							using JEnumInitializer = JinEngine::Core::JEnumInitializer;						\
							using JEnumInfo	= JinEngine::Core::JEnumInfo;									\
							using JEnumRegister	= JinEngine::Core::JEnumRegister<enumName>;					\
							static JEnumRegister jEnumRegister{J_STRINGIZE(enumName),						\
							J_STRINGIZE_ADD_COMMA(__VA_ARGS__), J_COUNT(__VA_ARGS__), false};						\
						}																					\
				}J_MERGE_NAME(enumName, EnumStruct);														\
			}																								\


namespace ReflectionData
		{

#define REGISTER_GUI_TABLE_GROUP(tableName, ...)																\
																										\
			inline static struct GuiTable##tableName													\
			{																							\
			public:																					\
				GuiTable##tableName()																\
				{																					\
					JinEngine::Core::JGuiExtraFunctionInfoMap::AddExtraFunctionInfo(std::make_unique<JinEngine::Core::JGuiTableInfo>(#tableName, __VA_ARGS__));	\
				}																					\
			}GuiTable##tableName;																	\


#define REGISTER_GUI_ENUM_CONDITION(conditioName, enumName, paramName)																\
																										\
			inline static struct GuiEnumCondition##conditioName												\
			{																							\
			public:																						\
				GuiEnumCondition##conditioName()																\
				{																						\
					JinEngine::Core::JGuiExtraFunctionInfoMap::AddExtraFunctionInfo(std::make_unique<JinEngine::Core::JGuiEnumConditionInfo>(#conditioName, #enumName, #paramName));	\
				}																						\
			}GuiEnumCondition##conditioName;																	\
		
		}

#pragma endregion

#pragma region Create GUI


#define GUI_TABLE_GROUP_USER(tableName, useColumnCount, canAddRowCount) std::make_unique<JinEngine::Core::JGuiTableUserInfo>(#tableName, useColumnCount, canAddRowCount)
#define GUI_ENUM_CONDITION_USER(conditionName, ...) std::make_unique<JinEngine::Core::JGuiEnumConditionUserInfo<J_COUNT(__VA_ARGS__)>>(#conditionName,  __VA_ARGS__)
#define GUI_ENUM_CONDITION_REF_USER(conditionName, ownerTypeParameterName, ...) std::make_unique<JinEngine::Core::JGuiEnumConditionRefUserInfo<J_COUNT(__VA_ARGS__)>>(#conditionName, #ownerTypeParameterName, __VA_ARGS__)

#define GUI_INPUT(isEnterToReturn, ...)	std::make_unique<JinEngine::Core::JGuiInputInfo>(isEnterToReturn, JinEngine::Core::J_PARAMETER_TYPE::UnKnown, __VA_ARGS__)
#define GUI_FIXED_INPUT(isEnterToReturn, fixedType, ...)	std::make_unique<JinEngine::Core::JGuiInputInfo>(isEnterToReturn, fixedType, __VA_ARGS__)
#define GUI_CHECKBOX(...)	std::make_unique<JinEngine::Core::JGuiCheckBoxInfo>(__VA_ARGS__)
#define GUI_SLIDER(minValue, maxValue, ...)	std::make_unique<JinEngine::Core::JGuiSliderInfo>(minValue, maxValue, __VA_ARGS__)
#define GUI_COLOR_PICKER(hasRgbInput, ...)	std::make_unique<JinEngine::Core::JGuiColorPickerInfo>(hasRgbInput, __VA_ARGS__)
#define GUI_SELECTOR(imageLevel, hasSizeSlider, ...)	std::make_unique<JinEngine::Core::JGuiSelectorInfo>(imageLevel, hasSizeSlider, __VA_ARGS__)
#define GUI_READONLY_TEXT(...)	std::make_unique<JinEngine::Core::JGuiReadOnlyTextInfo>(__VA_ARGS__) 
#define GUI_ENUM_COMBO(enumName, ...) std::make_unique<JinEngine::Core::JGuiEnumComboBoxInfo>(typeid(enumName).name(), __VA_ARGS__) 
#define GUI_LIST(listType, canDisplayElementGui, ...)  std::make_unique<JinEngine::Core::JGuiListInfo>(listType, canDisplayElementGui, __VA_ARGS__) 

#pragma endregion

#pragma region OPTION

#define SET_GUI_FLAG(flag) StaticTypeInfo().GetOption()->SetGuiWidgetFlag(flag)

#pragma endregion


	}
}