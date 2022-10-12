
#pragma once  
#include"../Editor/JEditorManager.h" 
#include<windows.h> 

class ApplicationStarter;
namespace JinEngine
{ 
	namespace Application
	{
		class JApplication
		{
		private:
			friend class ApplicationStarter;
		private:
			void Run();
			void RunProjectSelector();
			void RunEngine();
			void CalculateFrame();
		private:
			void StoreProject();
			void LoadProject();
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