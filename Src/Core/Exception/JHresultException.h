#pragma once
#include"JException.h"
#include<Windows.h>

namespace JinEngine
{
	namespace Core
	{
		class JHresultException: public JException
		{
		private:
			HRESULT hr;
		public:
			JHresultException(int line, const std::string file, HRESULT hr);
			const std::wstring what() const override;
			const std::wstring GetType() const override;
			HRESULT GetErrorCode() const;
			std::wstring GetErrorDescription()const;
		};
	}
}