#pragma once 
#include"JApplicationState.h"  

namespace JinEngine
{ 
	namespace Application
	{
		class JApplication;
		class JApplicationProject;
		class JApplicationEnginePrivate final
		{
		public:
			class AppAccess
			{
			private:
				friend class JApplication;
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