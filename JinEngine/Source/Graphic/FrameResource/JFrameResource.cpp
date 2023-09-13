#include"JFrameResource.h"
#include"../Buffer/JGraphicBuffer.h"

namespace JinEngine::Graphic
{
	void JFrameResource::CopyData(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const int elementIndex, const void* data)
	{
		auto base = GetGraphicBufferBase(type);
		if (base == nullptr)
			return;

		base->CopyData(elementIndex, data);
	}
}