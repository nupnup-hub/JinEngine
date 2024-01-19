#include"JFrameResource.h"
#include"../Buffer/JGraphicBuffer.h"

namespace JinEngine::Graphic
{
	uint JFrameResource::GetLocalLightCount()const noexcept
	{
		return GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::POINT_LIGHT) +
			GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::SPOT_LIGHT) +
			GetElementCount(J_UPLOAD_FRAME_RESOURCE_TYPE::RECT_LIGHT);
	}
	void JFrameResource::CopyData(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const int elementIndex, const void* data)
	{
		auto base = GetGraphicBufferBase(type);
		if (base == nullptr)
			return;

		base->CopyData(elementIndex, data);
	}
}