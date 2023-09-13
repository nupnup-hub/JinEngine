#pragma once 
#include"../Interface/JEditorObjectHandleInterface.h"
#include"../../Editor/Page/JEditorPageEnum.h"
#include"../../Core/JCoreEssential.h" 
#include"../../Core/Reflection/JGuiWidgetInfo.h"
#include"../../Core/Reflection/JGuiWidgetType.h"
#include<vector>
#include<unordered_map>
#include<string> 
#include<memory>

namespace JinEngine
{
	namespace Core
	{
		class Core::JIdentifier;
		class JTypeInfo;  
		class JGuiWidgetInfoHandleBase;
	}
	namespace Editor
	{
		class JEditorWindow;
		//widget need info 
		//info is declared reflect system
		class JReflectionGuiWidgetHelper
		{
		private:
			const size_t guid;
			JEditorWindow* ownerWnd;
		public:
			JReflectionGuiWidgetHelper(JEditorWindow* ownerWnd);
			~JReflectionGuiWidgetHelper();
		public:
			void UpdateGuiWidget(const Core::JUserPtr<Core::JIdentifier>& obj, Core::JTypeInfo* typeInfo);
			void Clear();
		};

		using JRGWH = JReflectionGuiWidgetHelper;
	}
}