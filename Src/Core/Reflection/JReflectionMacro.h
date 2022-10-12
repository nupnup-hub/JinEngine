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

#define REGISTER_TYPE_OPTION(...)
#define REGISTER_PROPERTY_OPTION(...)
#define REGISTER_METHOD_OPTION(...)

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


#define REGISTER_PROPERTY(propertyName, ...)														\
			namespace ReflectionData																\
			{																							\
				template<typename Class, typename Field, typename Pointer, Pointer ptr>					\
				class JinEngine::Core::JPropertyRegister;												\
																										\
				inline static struct propertyName##PropertyStruct										\
				{																						\
					public:																				\
						propertyName##PropertyStruct()													\
						{																				\
							static JinEngine::Core::JPropertyRegister<ThisType,							\
							decltype(propertyName),														\
							decltype(&ThisType::propertyName),											\
							&ThisType::propertyName> jPropertyRegister{#propertyName};					\
						}																				\
				}propertyName##Property;																\
			}																							\


#define REGISTER_METHOD(methodName, ...)																\
			namespace ReflectionData																	\
			{																							\
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
							jMethodInfoRegisterHelper{#methodName, #methodName};							\
						}																					\
				}methodName##Method;																		\
			}																								\


#define REGISTER_OVERLAOD_METHOD(methodName, ret, ...)														\
			namespace ReflectionData																		\
			{																								\
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
			}																								\


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
							J_STRINGIZE_ADD_COMMA(__VA_ARGS__), J_COUNT(__VA_ARGS__)};						\
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
							J_STRINGIZE_ADD_COMMA(__VA_ARGS__), J_COUNT(__VA_ARGS__)};						\
						}																					\
				}J_MERGE_NAME(enumName, EnumStruct);														\
			}																								\


	}
}