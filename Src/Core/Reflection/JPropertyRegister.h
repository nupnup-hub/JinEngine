#pragma once 
#include"JPropertyInfo.h" 

namespace JinEngine
{
	namespace Core
	{
		template<typename Type, typename Field, typename Pointer, Pointer ptr>
		class JPropertyRegister
		{
		public:
			JPropertyRegister(const std::string& name)
			{
				if constexpr (std::is_member_pointer_v<Pointer>)
				{
					static MemberPropertyHandler<Type, Field> handler{ ptr };
					static JPropertyInfo jPropertyInfo{ JPropertyInfoInitializer<Field>(name, handler, Type::StaticTypeInfo()) };
					static bool init = false;
					if (!init)
					{
						Type::StaticTypeInfo().AddPropertyInfo(&jPropertyInfo);
						init = true;
					}
				}
				else
				{
					static StaticPropertyHandler<Type, Field> handler{ ptr };
					static JPropertyInfo jPropertyInfo{ JPropertyInfoInitializer<Field>(name, handler, Type::StaticTypeInfo()) };
					static bool init = false;
					if (!init)
					{
						Type::StaticTypeInfo().AddPropertyInfo(&jPropertyInfo);
						init = true;
					}
				}
			}
		};
	}
}