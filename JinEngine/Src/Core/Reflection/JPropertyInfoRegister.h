#pragma once 
#include"JPropertyInfo.h"   

namespace JinEngine
{
	namespace Core
	{
		class JGuiWidgetInfo;
		template<typename Type> class JPropertyGuiWidgetInfoHandle;

		template<typename Type, typename Field, typename Pointer, Pointer ptr>
		class JPropertyInfoRegister
		{
		public:
			 JPropertyInfoRegister(const std::string& name)
			{
				static JPropertyOptionInfo optInfo{};
				if constexpr (std::is_member_pointer_v<Pointer>)
				{
					static JMemberPropertyHandler<Type, Field> handler{ ptr };
					static JPropertyInfo jPropertyInfo{ JPropertyInfoInitializer<Field>(name, handler, Type::StaticTypeInfo(), optInfo) };
					static bool init = false;
					if (!init)
					{
						Type::StaticTypeInfo().AddPropertyInfo(&jPropertyInfo);
						init = true;
					}
				}
				else
				{
					static JStaticPropertyHandler<Type, Field> handler{ ptr };
					static JPropertyInfo jPropertyInfo{ JPropertyInfoInitializer<Field>(name, handler, Type::StaticTypeInfo(), std::move(optInfo)) };
					static bool init = false;
					if (!init)
					{
						Type::StaticTypeInfo().AddPropertyInfo(&jPropertyInfo);
						init = true;
					}
				}
			}
			template<typename ...Widget>
			JPropertyInfoRegister(const std::string& name, Widget&&... var)
			{
				static JPropertyOptionInfo optInfo{};
				if constexpr (std::is_member_pointer_v<Pointer>)
				{
					static JMemberPropertyHandler<Type, Field> handler{ ptr };
					static JPropertyInfo jPropertyInfo{ JPropertyInfoInitializer<Field>(name, handler, Type::StaticTypeInfo(), optInfo) };
					static bool init = false;
					if (!init)
					{
						Type::StaticTypeInfo().AddPropertyInfo(&jPropertyInfo);
						Type::StaticTypeInfo().GetOption()->AddGuiWidgetInfoHandle(std::make_unique<JPropertyGuiWidgetInfoHandle<Field>>(&jPropertyInfo, std::forward<Widget>(var)...));
						init = true;
					}
				}
				else
				{
					static JStaticPropertyHandler<Type, Field> handler{ ptr };
					static JPropertyInfo jPropertyInfo{ JPropertyInfoInitializer<Field>(name, handler, Type::StaticTypeInfo(), std::move(optInfo)) };
					static bool init = false;
					if (!init)
					{
						Type::StaticTypeInfo().AddPropertyInfo(&jPropertyInfo);
						Type::StaticTypeInfo().GetOption()->AddGuiWidgetInfoHandle(std::make_unique<JPropertyGuiWidgetInfoHandle<Field>>(&jPropertyInfo, std::forward<Widget>(var)...));
						init = true;
					}
				}
			}
		};

		template<typename Type, typename Field, typename Pointer, Pointer ptr>
		class JPropertyExInfoRegister
		{
		public:
			template<typename GetPointer, typename SetPointer>
			JPropertyExInfoRegister(const std::string& name, GetPointer getPtr, SetPointer setPtr)
			{
				static JPropertyOptionInfo optInfo{};
				if constexpr (std::is_member_pointer_v<Pointer>)
				{
					static JMemberIndirectPropertyHandler<Type, Field, decltype(getPtr), decltype(setPtr)> handler{ getPtr, setPtr };
					static JPropertyInfo jPropertyInfo{ JPropertyInfoInitializer<Field>(name, handler, Type::StaticTypeInfo(), optInfo) };
					static bool init = false;
					if (!init)
					{
						Type::StaticTypeInfo().AddPropertyInfo(&jPropertyInfo);
						init = true;
					}
				}
				else
				{
					static JStaticIndirectPropertyHandler<Type, Field, decltype(getPtr), decltype(setPtr)> handler{ getPtr, setPtr };
					static JPropertyInfo jPropertyInfo{ JPropertyInfoInitializer<Field>(name, handler, Type::StaticTypeInfo(), optInfo) };
					static bool init = false;
					if (!init)
					{
						Type::StaticTypeInfo().AddPropertyInfo(&jPropertyInfo);
						init = true;
					}
				}

			}
			template<typename GetPointer, typename SetPointer, typename ...Widget>
			JPropertyExInfoRegister(const std::string& name, GetPointer getPtr, SetPointer setPtr, Widget&&... var)
			{
				static JPropertyOptionInfo optInfo{};
				if constexpr (std::is_member_pointer_v<Pointer>)
				{
					static JMemberIndirectPropertyHandler<Type, Field, decltype(getPtr), decltype(setPtr)> handler{ getPtr, setPtr };
					static JPropertyInfo jPropertyInfo{ JPropertyInfoInitializer<Field>(name, handler, Type::StaticTypeInfo(), optInfo) };
					static bool init = false;
					if (!init)
					{
						Type::StaticTypeInfo().AddPropertyInfo(&jPropertyInfo);
						Type::StaticTypeInfo().GetOption()->AddGuiWidgetInfoHandle(std::make_unique<JPropertyGuiWidgetInfoHandle<Field>>(&jPropertyInfo, std::forward<Widget>(var)...));
						init = true;
					}
				}
				else
				{
					static JStaticIndirectPropertyHandler<Type, Field, decltype(getPtr), decltype(setPtr)> handler{ getPtr, setPtr };
					static JPropertyInfo jPropertyInfo{ JPropertyInfoInitializer<Field>(name, handler, Type::StaticTypeInfo(), optInfo) };
					static bool init = false;
					if (!init)
					{
						Type::StaticTypeInfo().AddPropertyInfo(&jPropertyInfo);
						Type::StaticTypeInfo().GetOption()->AddGuiWidgetInfoHandle(std::make_unique<JPropertyGuiWidgetInfoHandle<Field>>(&jPropertyInfo, std::forward<Widget>(var)...));
						init = true;
					}
				}

			}

		};
	}
}