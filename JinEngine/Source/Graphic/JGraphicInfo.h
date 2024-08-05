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
#include"../Core/JCoreEssential.h"   
#include"GraphicResource/JGraphicResourceType.h"
#include"FrameResource/JFrameResourceType.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JGraphicInfo
		{  
		public:
			enum class TYPE
			{
				FRAME,
				RESOURCE,
				COUNT
			};
		public:
			static constexpr uint minCapacity = 4; 
		public:
			struct FrameResourceInfo
			{
			public:
				uint count[(uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT];
				uint capacity[(uint)J_FRAME_RESOURCE_UPLOAD_TYPE::COUNT];
			public:
				uint threadCount = 0;
			public:
				int currIndex = 0;
			public:
				FrameResourceInfo();
			public:
				uint GetCount(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept;
				uint GetCapacity(const J_FRAME_RESOURCE_UPLOAD_TYPE type)const noexcept;
				uint GetLocalLightCapacity()const noexcept;
			};
			struct GraphicResourceInfo
			{
			public:
				uint count[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
				uint border[(uint)J_GRAPHIC_RESOURCE_TYPE::COUNT];
			public:
				uint occlusionWidth = 0;
				uint occlusionHeight = 0;
				uint occlusionMinSize = 0;
				uint occlusionMapCount = 0;
				uint occlusionMapCapacity = 0;
			public:
				GraphicResourceInfo();
			public:
				uint GetCount(const J_GRAPHIC_RESOURCE_TYPE type)const noexcept;
				uint GetBorder(const J_GRAPHIC_RESOURCE_TYPE type)const noexcept;
			};
		public:
			FrameResourceInfo frame;
			GraphicResourceInfo resource; 
		public:
			//window size
			int width = 0;
			int height = 0;  
		public:
			void Load();
			void Store();
		};
	}
}