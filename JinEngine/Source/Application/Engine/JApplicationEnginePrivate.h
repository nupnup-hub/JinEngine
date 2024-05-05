#pragma once 
#include"../JApplicationType.h"  

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
				static void SetApplicationSubState(const J_APPLICATION_SUB_STATE newSubState)noexcept;
			private:
				static void EnterUpdateLoop();
				static void ExitUpdateLoop();
			};
		};
		//using JApplicationVariable = Application::JApplicationVariable; 
	}
	using JApplicationEnginePrivate = Application::JApplicationEnginePrivate;
}