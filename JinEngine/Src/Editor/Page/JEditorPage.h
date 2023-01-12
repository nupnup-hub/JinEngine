#pragma once 
#include"JEditor.h" 
#include"JEditorPageEnum.h" 
#include"../JEditorUpdateCondition.h"
#include"../../Core/Pointer/JOwnerPtr.h"

namespace JinEngine
{ 
	class JObject;
	namespace Editor
	{ 
		class JEditorAttribute; 
		class JEditorWindow;
		class JEditorPage : public JEditor
		{ 
		protected:
			struct WindowInitInfo
			{
			public:
				std::string name;
				float initWidthRate;
				float initHeightRate;
				float initPosXRate;
				float initPosYRate;
				bool isOpen;
				bool isLastAct;
			public:
				WindowInitInfo(const std::string name,
					float initWidthRate, float initHeightRate,
					float initPosXRate, float initPosYRate);
			public:
				std::string GetName()const noexcept;
				std::unique_ptr<JEditorAttribute> MakeAttribute()noexcept;
			};
		private:
			using OpenEditorWindowF = Core::JSFunctorType<void, JEditorPage&, const std::string>;
			using OpenSimpleWindowF = Core::JSFunctorType<void, bool*>;
		protected:  
			std::vector<JEditorWindow*>windows;
			std::vector<JEditorWindow*>opendWindow;
			JEditorWindow* focusWindow; 
			bool isPageOpen;
			const J_EDITOR_PAGE_FLAG pageFlag;
		private:
			std::unique_ptr<OpenEditorWindowF::Functor> openEditorWindowFunctor;
			std::unique_ptr<OpenSimpleWindowF::Functor> openSimpleWindowFunctor;
		public:
			JEditorPage(const std::string name, std::unique_ptr<JEditorAttribute> attribute, const J_EDITOR_PAGE_FLAG pageFlag);
			~JEditorPage();
			JEditorPage(const JEditorPage& rhs) = delete;
			JEditorPage& operator=(const JEditorPage& rhs) = delete;
		public:
			J_EDITOR_PAGE_FLAG GetPageFlag()const noexcept; 
			virtual J_EDITOR_PAGE_TYPE GetPageType()const noexcept = 0;
			virtual void SetInitWindow() = 0;
		public: 
			virtual void Initialize() = 0;
			virtual void UpdatePage(const JEditorPageUpdateCondition& condition) = 0;
			void EnterPage(int windowFlag)noexcept;
			void ClosePage()noexcept; 
		public:
			virtual bool IsValidOpenRequest(const Core::JUserPtr<JObject>& selectedObj) noexcept = 0;
		public:
			void OpenWindow(const std::string& windowname)noexcept;
			void OpenWindow(JEditorWindow* window)noexcept;
			void CloseWindow(const std::string& windowname)noexcept;
			void CloseWindow(JEditorWindow* window)noexcept; 
			void ActivateWindow(JEditorWindow* window)noexcept;
			void DeActivateWindow(JEditorWindow* window)noexcept;
			void FocusWindow(JEditorWindow* window)noexcept;
			void UnFocusWindow(JEditorWindow* window)noexcept;
		protected:
			OpenEditorWindowF::Functor* GetOpEditorWindowFunctorPtr()noexcept;
			OpenSimpleWindowF::Functor* GetOpSimpleWindowFunctorPtr()noexcept;
		protected:
			void UpdateDockSpace(const int dockspaceFlag); 
			void PrintOpenWindowState();
		protected:
			void DoSetOpen()noexcept override;
			void DoSetClose()noexcept override; 
			void DoActivate()noexcept override;
			void DoDeActivate()noexcept override; 
		private:
			JEditorWindow* FindEditorWindow(const std::string& windowName)const noexcept;
			bool IsWindowInVector(JEditorWindow* window, std::vector<JEditorWindow*>::const_iterator st, const uint count)const noexcept;
			int FindWindowIndex(JEditorWindow* window, std::vector<JEditorWindow*>::const_iterator st, const uint count)const noexcept;
		public:
			virtual void StorePage(std::wofstream& stream);
			virtual void LoadPage(std::wifstream& stream);
		};
	}
}