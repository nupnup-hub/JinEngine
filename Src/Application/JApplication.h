#pragma once 
#include"JApplicationVariable.h" 
#include"../Window/Editor/EditorManager.h"
#include"../ProjectSelector/ProjectSelector.h"

namespace JinEngine
{
	class JScene; 
	namespace Application
	{
		class JApplication
		{
		public:
			JApplication(HINSTANCE hInstance, const char* commandLine);
			JApplication(const JApplication& rhs) = delete;
			JApplication& operator=(const JApplication& rhs) = delete;

			~JApplication();
			void Run();
		private:
			void RunProjectSelector();
			void RunEngine();
			void CalculateFrame();
		private:
			JApplicationVariable applicationVar;
			ProjectSelector projectSelector;
			EditorManager editorManager;
		};
	}
}