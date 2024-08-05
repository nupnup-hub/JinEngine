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

#include"JGraphicModuleUtility.h"

namespace JinEngine
{
	void JGraphicModuleUtility::SetFrameDirty(JGraphicModuleManagedDataFrame* graphicData) noexcept
	{
		auto ptr = graphicData->GetFrameUpdateUserInterface();
		if (ptr == nullptr)
			return;

		ptr->SetFrameDirty();
	}
	void JGraphicModuleUtility::CreateFrame(JGraphicModuleManagedDataFrame* graphicData,
		const J_FRAME_RESOURCE_UPLOAD_TYPE type,
		const size_t areaGuid,
		const uint indexSize)
	{
		JFrameUploadDataCreationDesc desc(type, areaGuid, indexSize);
		GMI()->CreateFrameUploadData(graphicData, desc);
	}
	void JGraphicModuleUtility::DestroyFrame(JGraphicModuleManagedDataFrame* graphicData, const J_FRAME_RESOURCE_UPLOAD_TYPE type)
	{
		GMI()->DestroyFrameUploadData(graphicData, type);
	}
}