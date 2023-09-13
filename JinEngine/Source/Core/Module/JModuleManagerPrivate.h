#pragma once


namespace JinEngine
{ 
	class JMain;
	namespace Core
	{
		class JModuleManagerPrivate
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