#pragma once

namespace JinEngine
{ 
	class JMain;
	class JResourceManagerPrivate
	{
	public:
		class MainAccess
		{
		private:
			friend class JMain;
		private:
			static void Initialize();
			static void Terminate(const bool storeResource);
			static void StoreProjectResource();
			static void LoadSelectorResource();
			static void LoadProjectResource();
		};
	};
}