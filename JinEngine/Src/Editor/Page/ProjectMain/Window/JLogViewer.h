#pragma once
#include"../../JEditorWindow.h" 
#include"../../../Helpers/JEditorTabBarHelper.h"

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
			static constexpr uint tabItemCount = 3;
			std::unique_ptr<JEditorTabBarHelper<tabItemCount>> tabBarHelper;
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
			//¹Ì±¸Çö
			std::vector<Core::JLogBase*> GetAllLog();
			std::vector<Core::JLogBase*> GetUserLog();
			std::vector<Core::JLogBase*> GetTransitionLog();
		};
	}
}
