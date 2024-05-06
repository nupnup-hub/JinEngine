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
#include"../../Graphic/GraphicResource/JGraphicResourceType.h" 
#include"../../Graphic/JGraphicConstants.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceUserAccess;
	}
	namespace Editor
	{
		struct JGuiImageInfo
		{
		public:
			using ExtraFuncPerImagePtr = void(*)(int);
		public:
			Graphic::JGraphicResourceUserAccess* gUserAccess = nullptr;
			ExtraFuncPerImagePtr extraPerImagePtr = nullptr;	//excute after display valid image
		public:
			Graphic::ResourceHandle handle = nullptr;	//unsafe
		public:
			Graphic::J_GRAPHIC_RESOURCE_TYPE rType;
			Graphic::J_GRAPHIC_BIND_TYPE bType = Graphic::J_GRAPHIC_BIND_TYPE::SRV;
			uint dataIndex = 0;
			uint bIndex = 0; 
		public:
			bool displayAllType = false;
			bool useFirstHandle = false;	//handle을 사용할경우 반드시 true여야 한다.
		public:
			JGuiImageInfo(Graphic::JGraphicResourceUserAccess* gUserAccess);
			JGuiImageInfo(Graphic::JGraphicResourceUserAccess* gUserAccess, const Graphic::J_GRAPHIC_RESOURCE_TYPE rType);
			JGuiImageInfo(Graphic::JGraphicResourceUserAccess* gUserAccess, const Graphic::J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_BIND_TYPE bType);
			JGuiImageInfo(Graphic::ResourceHandle handle);
		public:
			bool IsValid()const noexcept;
		};
	}
}

