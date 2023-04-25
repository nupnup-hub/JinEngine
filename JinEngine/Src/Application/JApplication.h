#pragma once  
#include<windows.h> 
#include<memory>

class ApplicationStarter;
namespace JinEngine
{ 
	namespace Application
	{
		class JApplication
		{
		private:
			friend class ApplicationStarter;
			class JApplicationImpl;
		private:
			std::unique_ptr<JApplicationImpl> impl;
		private:
			void Run();
		private:
			JApplication(HINSTANCE hInstance, const char* commandLine);
			JApplication(const JApplication& rhs) = delete;
			JApplication& operator=(const JApplication& rhs) = delete;
			~JApplication();
		};
	}
}