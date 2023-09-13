#pragma once
#include"../../JEditorWindow.h" 
#include"../../../EditTool/JEditorTabBarHelper.h"

namespace JinEngine
{ 
	namespace Core
	{
		class JLogBase;
	}
	namespace Editor
	{
		template<int itemCount> class JEditorTabBarHelper;
		class JLogViewer final : public JEditorWindow
		{  
		private:
			using GetLogVecPtr = std::vector<Core::JLogBase*>(*)();
			using ClearLogHandlerPtr = void(*)();
		private:
			static constexpr uint tabItemCount = 4;
			std::unique_ptr<JEditorTabBarHelper<tabItemCount>> tabBarHelper;
		private:
			GetLogVecPtr getLogVecPtr[tabItemCount];
			ClearLogHandlerPtr clearLogHandlerPtr[tabItemCount];
		private: 
			int selectedTabIndex = 0;
			int selectedLogIndex = invalidIndex;
		public:
			JLogViewer(const std::string& name, 
				std::unique_ptr<JEditorAttribute> attribute,
				const J_EDITOR_PAGE_TYPE ownerPageType,
				const J_EDITOR_WINDOW_FLAG windowFlag);
			~JLogViewer() = default;
			JLogViewer(const JLogViewer& rhs) = delete;
			JLogViewer& operator=(const JLogViewer& rhs) = delete;
		public:
			J_EDITOR_WINDOW_TYPE GetWindowType()const noexcept final;
		public:
			void Initialize()noexcept;
			void UpdateWindow()final;
		private:
			void BuildLogViewer(); 
		private: 
			void DoActivate()noexcept final;
			void DoDeActivate()noexcept final;
		};
	}
}
