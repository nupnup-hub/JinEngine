#pragma once

namespace JinEngine
{
	namespace Application
	{
		class JApplication;
	} 
	class JResourceManagerPrivate
	{
	public:
		class ApplicationAccess
		{
		private:
			friend class Application::JApplication;
		private:
			static void Initialize();
			static void Terminate();
			static void StoreProjectResource();
			static void LoadSelectorResource();
			static void LoadProjectResource();
		};
	};
}