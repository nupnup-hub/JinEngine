#pragma once
#include"JException.h"
#include<Windows.h>

namespace JinEngine
{
	namespace Core
	{
		class JGraphicException final : public JException
		{
		private:
			HRESULT hr;
			HRESULT gfxHr;
			std::string hexerrorcode;
		public:
			JGraphicException(int line, const std::string file, HRESULT hr);
			const std::wstring what() const final;
			const std::wstring GetType()const final;
		};
	}
}