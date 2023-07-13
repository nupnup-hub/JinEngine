#pragma once


namespace JinEngine
{
	namespace Application
	{
		class JApplication;
	}
	namespace Core
	{
		class JModuleManagerPrivate
		{
		public:
			class AppInterface
			{
			private:
				friend class Application::JApplication;
			private: 
				static void Clear();
			};
		};
	}
}