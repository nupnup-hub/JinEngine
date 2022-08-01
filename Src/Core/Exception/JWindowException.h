#pragma once 
#include"JException.h"
#include<Windows.h>

namespace JinEngine
{
	namespace Core
	{
		class JWindowException : public JException
		{
		private:
			DWORD lastError;
		public:
			JWindowException(int line, const std::string file);
			const std::wstring what() const override;
			const std::wstring GetType() const override;
		};
	}
}