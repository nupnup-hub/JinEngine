#pragma once
#include"../../Core/Exception/JException.h"
#include<Windows.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicException final : public Core::JException
		{
		private:
			HRESULT hr;
			std::wstring hexerrorcode;
			std::wstring errorMsg;
		public:
			JGraphicException(int line, const std::string file, HRESULT hr);
			const std::wstring what() const final;
			const std::wstring GetType()const final;
		};
	}
}

#define ThrowIfFailedG(hr)                                                       \
{                                                                                \
    HRESULT hr__ = (hr);                                                         \
    if(hr__ != S_OK) {throw JGraphicException(__LINE__, __FILE__, hr__); }       \
}																				 \

#define ReturnIfFailedG(hr, ret)                                                 \
{                                                                                \
    HRESULT hr__ = (hr);                                                         \
    if(hr__ != S_OK) {JGraphicException(__LINE__, __FILE__, hr__); return ret;}  \
}																				 \

