#pragma once
#include"../Pointer/JOwnerPtr.h" 

namespace JinEngine
{
	namespace Core
	{
		class JPluginInterface;
		class JPluginManager;
		class JPluginPrivate
		{
		public:
			class IOInterface
			{
			private:
				friend class JPluginManager;
			private:
				static JOwnerPtr<JPluginInterface> LoadPlugin(const std::wstring& metafilePath);
				static bool StorePlugin(JPluginInterface* p);
			};
		};
	}
}