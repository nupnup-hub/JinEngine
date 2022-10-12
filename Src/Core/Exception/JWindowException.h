#pragma once 
#include"JException.h"
#include<Windows.h>

namespace JinEngine
{
	namespace Core
	{
		class JWindowException final : public JException
		{
		private:
			DWORD lastError;
		public:
			JWindowException(int line, const std::string file);
			const std::wstring what() const final;
			const std::wstring GetType() const final;
		};
	}
}