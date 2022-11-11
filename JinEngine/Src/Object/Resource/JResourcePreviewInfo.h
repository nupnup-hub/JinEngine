#pragma once  
#include<string>
#include"JResourceType.h"
#include"../../Core/JDataType.h"

namespace JinEngine
{ 
	class JResourcePreviewInfo
	{
	public:
		uint st = 0;
		uint count = 0;
		const uint heapOffset;
		const uint capacity;
	public:
		JResourcePreviewInfo(unsigned int heapOffset, unsigned int capacity);
		bool IsResourcePreviewFormat(const std::wstring& format);
		void Enter();
	};
}