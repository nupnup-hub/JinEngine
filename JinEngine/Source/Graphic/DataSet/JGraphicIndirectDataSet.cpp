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


#include"JGraphicIndirectDataSet.h"

namespace JinEngine::Graphic
{
	JGraphicBlurTaskSettingSet::JGraphicBlurTaskSettingSet(ResourceHandle from, ResourceHandle to, std::unique_ptr<JBlurDesc>&& desc)
		:fromRH(from), toRH(to), fromDH(nullptr), toDH(nullptr), useDataHandle(false), desc(std::move(desc))
	{}
	JGraphicBlurTaskSettingSet::JGraphicBlurTaskSettingSet(const Core::JDataHandle* from, const Core::JDataHandle* to, std::unique_ptr<JBlurDesc>&& desc)
		: fromRH(nullptr), toRH(nullptr), fromDH(from), toDH(to), useDataHandle(true), desc(std::move(desc))
	{}

	JGraphicMipmapGenerationSettingSet::JGraphicMipmapGenerationSettingSet(const std::vector<Core::JDataHandle>& mipMapHandle, std::unique_ptr<JDownSampleDesc>&& desc)
		: mipMapHandle(mipMapHandle), desc(std::move(desc))
	{}

	JGraphicConvetColorSettingSet::JGraphicConvetColorSettingSet(JGraphicResourceInfo* from, JGraphicResourceInfo* to, const JConvertColorDesc& desc)
		: from(from), to(to), desc(desc)
	{}
}
