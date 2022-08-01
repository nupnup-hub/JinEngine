#pragma once
#include"JException.h" 

namespace JinEngine
{
	namespace Core
	{
		class JNormalException : public JException
		{
		public:
			JNormalException(int line, const std::string file, const std::string msg = "");
			const std::wstring what() const override;
			const std::wstring GetType() const override;
		};
	}
}