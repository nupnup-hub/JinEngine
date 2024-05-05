#pragma once
#include<exception> 
#include <sstream>
#include<Windows.h>
#include"../JCoreEssential.h"

namespace JinEngine
{
	namespace Core
	{
		class JException
		{
		protected:
			mutable std::wstring whatBuffer;
		private:
			int line = 0;
			std::wstring file = L" ";
		public:
			JException(int line, const std::wstring& file);
			virtual ~JException();
			virtual const std::wstring what() const;
			virtual const std::wstring GetType() const;
			int GetLine() const;
			const std::wstring GetFile() const;
			const std::wstring GetOriginString() const;
		public:
			static std::wstring TranslateErrorCode(HRESULT hr);
		};
	}
}
 
