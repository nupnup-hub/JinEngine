/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


#pragma once 
#include"JPropertyInfo.h"   
#include"../Pointer/JOwnerPtr.h"

namespace JinEngine
{
	namespace Core
	{
		class JGuiWidgetInfo;
		template<typename Type> class JPropertyGuiWidgetInfoHandle;

		template<typename Type, typename Field, typename Pointer, Pointer ptr>
		class JPropertyInfoRegister final
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
					static JPropertyInfo jPropertyInfo{JPropertyInfoInitializer<Field>(name, handler, Type::StaticTypeInfo(), std::move(optInfo)) };
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
						Type::StaticTypeInfo().GetOption()->AddGuiWidgetInfoHandle(JPtrUtil::MakeOwnerPtr<JPropertyGuiWidgetInfoHandle<Field>>(&jPropertyInfo, std::forward<Widget>(var)...));
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
						Type::StaticTypeInfo().GetOption()->AddGuiWidgetInfoHandle(JPtrUtil::MakeOwnerPtr<JPropertyGuiWidgetInfoHandle<Field>>(&jPropertyInfo, std::forward<Widget>(var)...));
						init = true;
					}
				}
			}
		};

		template<typename Type, typename Field, typename Pointer, Pointer ptr>
		class JPropertyExInfoRegister final
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
						Type::StaticTypeInfo().GetOption()->AddGuiWidgetInfoHandle(JPtrUtil::MakeOwnerPtr<JPropertyGuiWidgetInfoHandle<Field>>(&jPropertyInfo, std::forward<Widget>(var)...));
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
						Type::StaticTypeInfo().GetOption()->AddGuiWidgetInfoHandle(JPtrUtil::MakeOwnerPtr<JPropertyGuiWidgetInfoHandle<Field>>(&jPropertyInfo, std::forward<Widget>(var)...));
						init = true;
					}
				}

			}

		};
	}
}