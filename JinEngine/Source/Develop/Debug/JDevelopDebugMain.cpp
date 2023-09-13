#include"JDevelopDebugMain.h"
#include"JDevelopDebug.h"
namespace JinEngine::Develop
{
	void JDevelopDebugMain::Initialize()
	{

	}
	void JDevelopDebugMain::Clear()
	{
#ifdef DEVELOP
		JDevelopDebug::Clear();
#else
#endif
	}
}