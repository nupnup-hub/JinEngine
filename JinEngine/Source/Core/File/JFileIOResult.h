#pragma once
#include"../JCoreEssential.h"

namespace JinEngine
{
	namespace Core
	{
		enum class J_FILE_IO_RESULT
		{
			SUCCESS,
			FAIL_INVALID_DATA, 
			FAIL_DO_NOT_SAVE_DATA,
			FAIL_STREAM_ERROR, 
			WAIT,
		};

		struct JFileIOResultInfo
		{
		public:
			std::string name;
			J_FILE_IO_RESULT res;
		public:
			JFileIOResultInfo(const std::string name, const J_FILE_IO_RESULT res)
				:name(name),res(res)
			{}
		}; 
	}
}