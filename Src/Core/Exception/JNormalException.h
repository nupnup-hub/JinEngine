#pragma once
#include"JException.h" 

namespace JinEngine
{
	namespace Core
	{
		class JNormalException final : public JException
		{
		public:
			JNormalException(int line, const std::string file, const std::string msg = "");
			const std::wstring what() const final;
			const std::wstring GetType() const final;
		};
	}
}