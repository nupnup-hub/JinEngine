#pragma once
namespace JinEngine
{
	class JMain;
	namespace Develop
	{
		class JDevelopDebugMain
		{
		private:
			friend class JMain;
		private:
			static void Initialize();
			static void Clear();
		};
	}
}