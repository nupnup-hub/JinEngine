#pragma once
#include<string>
#include"../Pointer/JOwnerPtr.h"

namespace JinEngine
{
	namespace Core
	{
		class JModule;
		class JModuleManager;
		class JModulePrivate
		{
		public:
			class IOInterface
			{
			private:
				friend class JModuleManager;
			private:
				static JOwnerPtr<JModule> LoadModule(const std::wstring& metafilePath);
				static bool StoreModule(const JUserPtr<JModule>& m);
			};
		};
	}
}