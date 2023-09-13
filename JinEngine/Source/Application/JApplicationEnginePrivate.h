#pragma once 
#include"JApplicationState.h"  

namespace JinEngine
{ 
	class JMain;
	namespace Application
	{ 
		class JApplicationProject;
		class JApplicationEnginePrivate final
		{
		public:
			class MainAccess
			{
			private:
				friend class JMain;
			private:
				static void Initialize();
				static bool MakeEngineFolder();
			private:
				static void SetApplicationState(const J_APPLICATION_STATE newState)noexcept;
			};
		};
		//using JApplicationVariable = Application::JApplicationVariable; 
	}
	using JApplicationEnginePrivate = Application::JApplicationEnginePrivate;
}