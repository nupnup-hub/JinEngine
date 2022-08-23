#pragma once 
#include"JApplicationVariable.h" 
#include"../Editor/JEditorManager.h"
#include"../ProjectSelector/ProjectSelector.h"

class ApplicationStarter;
namespace JinEngine
{ 
	namespace Application
	{
		class JApplication
		{
		private:
			friend class ApplicationStarter;
		public:
			void Run();
		private:
			void RunProjectSelector();
			void RunEngine();
			void CalculateFrame();
		private:
			JApplication(HINSTANCE hInstance, const char* commandLine);
			JApplication(const JApplication& rhs) = delete;
			JApplication& operator=(const JApplication& rhs) = delete;
			~JApplication();
		private:
			JApplicationVariable applicationVar;
			ProjectSelector projectSelector;
			Editor::JEditorManager editorManager;
		};
	}
}