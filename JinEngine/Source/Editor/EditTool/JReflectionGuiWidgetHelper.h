#pragma once 
#include"../../Editor/Page/JEditorPageEnum.h"
#include"../../Object/JObjectModifyInterface.h"
#include"../../Core/Reflection/JGuiWidgetInfo.h"
#include"../../Core/Reflection/JGuiWidgetType.h"  
#include<stack>

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
		public:
			struct WidgetOption
			{
			public:
				float nameSpaceOffset = 0;
			};
		private:
			const size_t guid;
			JEditorWindow* ownerWnd; 
		private:
			bool isTableOpen = false;
		public:
			JReflectionGuiWidgetHelper(JEditorWindow* ownerWnd);
			~JReflectionGuiWidgetHelper();
		public:
			void BeginGuiWidget(const Core::JUserPtr<Core::JIdentifier>& obj, const size_t userGuid);
			void EndGuiWidget();
		public:
			JUserPtr<Core::JIdentifier> GetLastSelected()const noexcept;
		public:
			void UpdateGuiWidget(const Core::JUserPtr<Core::JIdentifier>& obj, Core::JTypeInfo* typeInfo);
			void Clear();
		};

		using JRGWH = JReflectionGuiWidgetHelper;
	}
}