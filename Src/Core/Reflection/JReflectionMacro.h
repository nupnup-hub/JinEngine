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

		}

#define REGISTER_CLASS(typeName, ...)																	\
																										\
		public:																							\
			using TypeDepth = 	JinEngine::Core::Depth<typeName>;										\
			using ParentType = typename JinEngine::Core::ParentClass<typeName>::Type;					\
			using ThisType = typeName;																	\
																										\
		protected:																						\
			using JTypeInfo = JinEngine::Core::JTypeInfo;										\
			using JTypeInfoInitializer = JinEngine::Core::JTypeInfoInitializer<typeName>;		\
			using JTypeInfoRegister = JinEngine::Core::JTypeInfoRegister<typeName>;				\
			using JReflectionInfo = JinEngine::Core::JReflectionInfo;							\
			using JPtrUtil = JinEngine::Core::JPtrUtil;											\
																								\
		private:																				\
			friend class JTypeInfoInitializer;													\
			template<typename T> friend class JinEngine::Core::JOwnerPtr;						\
			friend class JPtrUtil;																\
																								\
		public:																					\
			static JTypeInfo& StaticTypeInfo()													\
			{																					\
				static JTypeInfoRegister typeRegister{#typeName};								\
				return *JReflectionInfo::Instance().GetTypeInfo(#typeName);						\
			}																					\
																								\
			virtual JTypeInfo& GetTypeInfo()const {return typeInfo;}							\
			virtual int GetTypeDepth()const {return TypeDepth::value;}							\
			inline static std::string TypeName() {return #typeName;}							\
			inline static std::wstring TypeWName() {return L#typeName;}							\
																								\
		private:																				\
			inline static JTypeInfo& typeInfo = StaticTypeInfo();								\
																								\
																								\

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
			class JinEngine::Core::JPropertyInfoRegister;												\
																									\
			inline static struct propertyName##PropertyStruct										\
			{																						\
				public:																				\
					propertyName##PropertyStruct()													\
					{																				\
						static JinEngine::Core::JPropertyInfoRegister<ThisType,							\
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
	
		}


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

#define REGISTER_GUI_TABLE_GROUP(groupName, ...)																\
																										\
			inline static struct GuiTable##groupName													\
			{																							\
			public:																					\
				GuiTable##groupName()																\
				{																					\
					JinEngine::Core::JGuiGroupMap::AddGuiGroup(std::make_unique<JinEngine::Core::JGuiTableInfo>(#groupName, __VA_ARGS__));	\
				}																					\
			}GuiTable##groupName;																	\


#define REGISTER_GUI_TRIGGER_GROUP(groupName, enumName, paramName)																\
																										\
			inline static struct GuiEnumTrigger##groupName												\
			{																							\
			public:																						\
				GuiEnumTrigger##groupName()																\
				{																						\
					JinEngine::Core::JGuiGroupMap::AddGuiGroup(std::make_unique<JinEngine::Core::JGuiEnumTriggerInfo>(#groupName, #enumName, #paramName));	\
				}																						\
			}GuiEnumTrigger##groupName;																	\

		
		}

#define GUI_INPUT(isEnterToReturn, ...)	std::make_unique<JinEngine::Core::JGuiInputInfo>(isEnterToReturn, __VA_ARGS__)
#define GUI_CHECKBOX(...)	std::make_unique<JinEngine::Core::JGuiCheckBoxInfo>(__VA_ARGS__)
#define GUI_SLIDER(minValue, maxValue, ...)	std::make_unique<JinEngine::Core::JGuiSliderInfo>(minValue, maxValue, __VA_ARGS__)
#define GUI_COLOR_PICKER(hasRgbInput, ...)	std::make_unique<JinEngine::Core::JGuiColorPickerInfo>(hasRgbInput, __VA_ARGS__)
#define GUI_SELECTOR(isImageRtTexture, hasSizeSlider, ...)	std::make_unique<JinEngine::Core::JGuiSelectorInfo>(isImageRtTexture, hasSizeSlider, __VA_ARGS__)
#define GUI_READONLY_TEXT(...)	std::make_unique<JinEngine::Core::JGuiReadOnlyTextInfo>(__VA_ARGS__) 
#define GUI_ENUM_COMBO(enumName, ...) std::make_unique<JinEngine::Core::JGuiEnumComboBoxInfo>(#enumName, __VA_ARGS__) 

#define GUI_GROUP_COMMON(groupKey) std::make_unique<JinEngine::Core::JGuiGroupMemberInfo>(#groupKey)
#define GUI_GROUP_ENUM_TRIGGER(groupKey, ...) std::make_unique<JinEngine::Core::JGuiEnumTriggerGroupMemberInfo<J_COUNT(__VA_ARGS__)>>(#groupKey, __VA_ARGS__)


	}
}