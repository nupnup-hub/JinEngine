#pragma once
#include"../GuiLibEx/JGuiWidgetType.h"
#include"../Interface/JEditorObjectInterface.h"
#include"../../Editor/Page/JEditorPageEnum.h"
#include"../../Core/JDataType.h" 
#include"../../Core/Reflection/JGuiWidgetInfo.h"
#include<vector>
#include<unordered_map>
#include<string> 
#include<memory>

namespace JinEngine
{
	namespace Core
	{
		class JIdentifier;
		class JTypeInfo;  
		class JGuiWidgetInfoHandleBase;
	}
	namespace Editor
	{
		//widget need info 
		//info is declared reflect system
		class JReflectionGuiWidgetHelper
		{
		private:
			const size_t guid;
			const J_EDITOR_PAGE_TYPE ownerPageType;
		public:
			JReflectionGuiWidgetHelper(const J_EDITOR_PAGE_TYPE ownerPageType);
			~JReflectionGuiWidgetHelper();
		public:
			void UpdateGuiWidget(Core::JIdentifier* obj, Core::JTypeInfo* typeInfo); 
			void Clear();
		};

		using JRGWH = JReflectionGuiWidgetHelper;
	}
}