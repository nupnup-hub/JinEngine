#include"JResourcePreviewInfo.h"

namespace JinEngine
{
	JResourcePreviewInfo::JResourcePreviewInfo(unsigned int heapOffset, unsigned int capacity)
		:heapOffset(heapOffset),capacity(capacity)
	{}
	bool JResourcePreviewInfo::IsResourcePreviewFormat(const std::wstring& format)
	{
		if (format == L".mat")
			return true;
		else if (format == L".fbx")
			return true;
		else if (format == L".obj")
			return true;
		else
			return false;
	}
	void JResourcePreviewInfo::Enter()
	{
		st = 0;
	}
}