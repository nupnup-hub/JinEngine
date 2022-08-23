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
			//��� �Լ��� �����ص�
			//basic template �μ��� specialize template�μ��� �ٸ��� Ư��ȭ ��������
			//basic == template<typename T, typename = int> �Ͻ� call<A> => call<A, int>���ȴ�.
			//�׷��Ƿ� std::void_t�� Ÿ���� void�� ����Ʈ ����
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