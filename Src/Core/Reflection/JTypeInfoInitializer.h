#pragma once
#include<type_traits>
#include<string> 

namespace JinEngine
{
	class JObject;
	namespace Core
	{
		namespace
		{	
			//멤버 함수가 존재해도
			//basic template 인수가 specialize template인수와 다르면 특수화 되지않음
			//basic == template<typename T, typename = int> 일시 call<A> => call<A, int>가된다.
			//그러므로 std::void_t에 타입인 void로 디폴트 설정
			template<typename T, typename = void>
			struct HasEngineDefinedRegister : std::false_type
			{};
			template<typename T>
			struct HasEngineDefinedRegister<T, std::void_t<decltype(&T::RegisterJFunc)>> : std::true_type
			{};
		}

		class JTypeInfo;
		template<typename Type>
		class JTypeInfoInitializer
		{
		private:
			friend class JTypeInfo;
		public:
			JTypeInfoInitializer(const std::string& name)
				:name(name)
			{
				fullName = typeid(Type).name();
				hashCode = typeid(Type).hash_code();
				if constexpr (std::is_void_v<Type::ParentType>)
					parent = nullptr;
				else
					parent = &Type::ParentType::StaticTypeInfo();

				if constexpr (HasEngineDefinedRegister<Type>::value)
					Type::RegisterJFunc();
			}
		private:
			std::string name;
			std::string fullName;
			size_t hashCode;
			JTypeInfo* parent;
		};
	}
} 