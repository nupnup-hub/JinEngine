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


#pragma once
#include"JGraphicDataSet.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceInfo;
		struct JGraphicBlurTaskSettingSet
		{
		public:
			ResourceHandle fromRH;
			ResourceHandle toRH;
		public:
			const Core::JDataHandle* fromDH;
			const Core::JDataHandle* toDH;
		public:
			std::unique_ptr<JBlurDesc> desc;
			std::unique_ptr<JGraphicBlurComputeSet> dataSet;	//out
		public:
			const bool useDataHandle = false;
		public:
			JGraphicBlurTaskSettingSet(ResourceHandle from, ResourceHandle to, std::unique_ptr<JBlurDesc>&& desc);
			JGraphicBlurTaskSettingSet(const Core::JDataHandle* from, const Core::JDataHandle* to, std::unique_ptr<JBlurDesc>&& desc);
		};

		struct JGraphicMipmapGenerationSettingSet
		{
		public:
			const std::vector<Core::JDataHandle>& mipMapHandle;
			std::unique_ptr<JDownSampleDesc> desc;
			std::unique_ptr<JGraphicDownSampleComputeSet> dataSet;		//out
		public:
			JGraphicMipmapGenerationSettingSet(const std::vector<Core::JDataHandle>& mipMapHandle, std::unique_ptr<JDownSampleDesc>&& desc);
		};

		struct JGraphicConvetColorSettingSet
		{
		public:
			std::unique_ptr<JGraphicConvertColorComputeSet> dataSet;		//out
		public: 
			JGraphicResourceInfo* from;											//in
			JGraphicResourceInfo* to;										//in
		public:
			JConvertColorDesc desc;											//in
		public:
			JGraphicConvetColorSettingSet(JGraphicResourceInfo* from, JGraphicResourceInfo* to, const JConvertColorDesc& desc);
		};

	}
}