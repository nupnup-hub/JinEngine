#pragma once
#include"JSync.h"

namespace JinEngine
{
	namespace Core
	{
#ifdef WINDOW
		using TlsIndex = DWORD;
#endif
		struct JTlsData
		{
		public:
			uint threadIndex = 0;
			JCriticalSectionTool* syncTool = nullptr;
		};
	}
}