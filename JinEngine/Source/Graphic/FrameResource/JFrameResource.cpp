/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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
	void JFrameResource::CopyData(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint elementIndex, const void* data)
	{
		auto base = GetGraphicBufferBase(type);
		if (base == nullptr)
			return;

		base->CopyData(elementIndex, data);
	}
	void JFrameResource::CopyData(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint elementIndex, const uint count, const void* data, const uint dataElementSize)
	{
		auto base = GetGraphicBufferBase(type);
		if (base == nullptr)
			return;

		base->CopyData(elementIndex, count, data, dataElementSize);
	}
}