#pragma once
#include"../GuiLibEx/JGuiWidgetType.h"
#include"../Interface/JEditorObjectInterface.h"
#include"../../Core/JDataType.h" 
#include"../../Core/Reflection/JReflectionGuiInfo.h"
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
		class JMethodInfo;
		class JPropertyInfo;
	}
	namespace Editor
	{
		//widget need info 
		//info is declared reflect system

		class JGuiPropertyWidgetHandle
		{ 
		private:
			bool isPrintTitle = true;
		public:
			virtual ~JGuiPropertyWidgetHandle() = default;
		public:
			virtual void Initialize(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo) = 0;
			virtual void Update(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo) = 0;
		public:
			bool IsPrintTitle()const noexcept;
		public:
			void SetPrintTitle(bool value)noexcept; 
		};

		class JGuiPropertyGroupHandle
		{
		public:
			virtual ~JGuiPropertyGroupHandle() = default;
		public:
			virtual void Update(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo, JGuiPropertyWidgetHandle* widgetHandle) = 0;
		};

		class JGuiMethodWidgetHandle
		{
		public:
			virtual ~JGuiMethodWidgetHandle() = default;
		public:
			virtual void Initialize(Core::JIdentifier* obj, Core::JMethodInfo* mInfo) = 0;
			virtual void Update(Core::JIdentifier* obj, Core::JMethodInfo* mInfo) = 0;
		};

		class JReflectionGuiWidgetHelper
		{
		private:
			std::unordered_map<std::string, std::unique_ptr<JGuiPropertyWidgetHandle>> guiPWidgetHandleMap;
			std::unordered_map<std::string, std::unique_ptr<JGuiMethodWidgetHandle>> guiMWidgetHandleMap;
			std::unordered_map<std::string, std::unique_ptr<JGuiPropertyGroupHandle>> guiPGroupHandleMap;
		public: 
			void UpdatePropertyGuiWidget(Core::JIdentifier* obj, Core::JPropertyInfo* pInfo);
			void UpdateMethodGuiWidget(Core::JIdentifier* obj, Core::JMethodInfo* mInfo);
		public:
			void Clear();
		};

		using JRGWH = JReflectionGuiWidgetHelper;
	}
}