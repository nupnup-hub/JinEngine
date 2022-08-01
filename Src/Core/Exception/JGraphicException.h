#pragma once
#include"JException.h"
#include<Windows.h>

namespace JinEngine
{
	namespace Core
	{
		class JGraphicException : public JException
		{
		private:
			HRESULT hr;
			HRESULT gfxHr;
			std::string hexerrorcode;
		public:
			JGraphicException(int line, const std::string file, HRESULT hr);
			const std::wstring what() const override;
			const std::wstring GetType()const override;
		};
	}
}