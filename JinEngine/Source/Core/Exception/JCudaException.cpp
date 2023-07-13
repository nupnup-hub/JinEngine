#include"JCudaException.h"
#include"../../Utility/JCommonUtility.h"
#include "cuda_runtime.h"

namespace JinEngine
{
	namespace Core
	{
		JCudaException::JCudaException(int line, const std::string file, JCudaError err)
			:JException(line, JCUtil::StrToWstr(file)), err(err)
		{
			whatBuffer = JCUtil::StrToWstr(cudaGetErrorString((cudaError)err));
		}
		const std::wstring JCudaException::what()const
		{
			return GetType() + L"\n "+ whatBuffer;
		}
		const std::wstring JCudaException::GetType()const
		{
			return L"Cuda Exception";
		}
	}
}