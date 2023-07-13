#pragma once
#include"JException.h"
#include<Windows.h>

namespace JinEngine
{
	namespace Core
	{
		class JCudaException final : public JException
		{
		private:
			using JCudaError = int;
		private:
			JCudaError err;
		public:
			JCudaException(int line, const std::string file, JCudaError err);
			const std::wstring what() const final;
			const std::wstring GetType()const final;
		};
	}
}