#pragma once  
#include"../Editor/JEditorManager.h" 
#include"../Core/Event/JEventListener.h"
#include"../Window/JWindowEventType.h"
#include<windows.h> 

class ApplicationStarter;
namespace JinEngine
{ 
	namespace Application
	{
		class JApplication : public Core::JEventListener<size_t, Window::J_WINDOW_EVENT>
		{
		private:
			friend class ApplicationStarter;
		private:
			const size_t guid;
		private:
			void Run();
			void RunProjectSelector();
			void RunEngine();
			void CalculateFrame();
		private:
			void StoreProject();
			void LoadProject();
		private:
			void OnEvent(const size_t& senderGuid, const Window::J_WINDOW_EVENT& eventType)final;
		private:
			JApplication(HINSTANCE hInstance, const char* commandLine);
			JApplication(const JApplication& rhs) = delete;
			JApplication& operator=(const JApplication& rhs) = delete;
			~JApplication();
		private: 
			Editor::JEditorManager editorManager;
		};
	}
}