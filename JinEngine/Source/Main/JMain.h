#pragma once  
#include"../Core/JCoreEssential.h"
#include<windows.h>  

class JMainEntry;
namespace JinEngine
{ 
	class JMain final
	{
	private:
		friend class JMainEntry;
		class JMainImpl;
	private:
		std::unique_ptr<JMainImpl> impl;
	private:
		void Run();
	private:
		JMain(HINSTANCE hInstance, const char* commandLine);
		JMain(const JMain& rhs) = delete;
		JMain& operator=(const JMain& rhs) = delete;
		~JMain();
	};
}