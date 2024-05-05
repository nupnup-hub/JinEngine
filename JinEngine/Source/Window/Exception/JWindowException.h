#pragma once 
#include"../../Core/Exception/JException.h"
#include<Windows.h>

namespace JinEngine
{
	namespace Window
	{
		class JWindowException final : public Core::JException
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

#define ThrowIfFailedW(b)                                                        \
{                                                                                \
    bool b_ = (b);                                                               \
    if(!b_) {throw JWindowException(__LINE__, __FILE__); }                 \
}																				 \

#define ReturnIfFailedW(b, ret)                                                  \
{                                                                                \
    bool b_ = (b);                                                               \
    if(!b_) {JWindowException(__LINE__, __FILE__); return ret;}                   \
}																	             \
 