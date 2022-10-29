#pragma once 
#include"JPropertyInfo.h"  

namespace JinEngine
{
	namespace Core
	{
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
			JPropertyInfoRegister(const std::string& name, std::unique_ptr<JGuiWidgetInfo>&& widgetInfo)
			{
				static JPropertyOptionInfo optInfo{ std::move(widgetInfo) };
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
			template<typename GetPointer, typename SetPointer>
			JPropertyInfoRegister(const std::string& name, GetPointer getPtr, SetPointer setPtr)
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
			template<typename GetPointer, typename SetPointer>
			JPropertyInfoRegister(const std::string& name, GetPointer getPtr, SetPointer setPtr, std::unique_ptr<JGuiWidgetInfo>&& widgetInfo)
			{ 
				static JPropertyOptionInfo optInfo{ std::move(widgetInfo) };
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

		};
	}
}