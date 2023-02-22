#pragma once
#include"../GuiLibEx/JGuiWidgetType.h"
#include"../Interface/JEditorObjectInterface.h"
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
		public:
			JReflectionGuiWidgetHelper();
			~JReflectionGuiWidgetHelper();
		public:
			void UpdateGuiWidget(Core::JIdentifier* obj, Core::JTypeInfo* typeInfo); 
			void Clear();
		};

		using JRGWH = JReflectionGuiWidgetHelper;
	}
}