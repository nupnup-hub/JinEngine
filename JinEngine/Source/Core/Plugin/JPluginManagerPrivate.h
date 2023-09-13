#pragma once
namespace JinEngine
{

	class JMain;
	namespace Core
	{
		class JPluginManagerPrivate
		{
		public:
			class MainAccess
			{
			private:
				friend class JMain;
			private:
				static void Clear();
			};
		};
	}
}