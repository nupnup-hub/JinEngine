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


#include"JGuiImageInfo.h"
#include"../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../Graphic/GraphicResource/JGraphicResourceUserAccess.h"

namespace JinEngine::Editor
{ 
	JGuiImageInfo::JGuiImageInfo(Graphic::JGraphicResourceUserAccess* gUserAccess)
		:gUserAccess(gUserAccess), useFirstHandle(true)
	{}
	JGuiImageInfo::JGuiImageInfo(Graphic::JGraphicResourceUserAccess* gUserAccess, const Graphic::J_GRAPHIC_RESOURCE_TYPE rType)
		: gUserAccess(gUserAccess), rType(rType)
	{}
	JGuiImageInfo::JGuiImageInfo(Graphic::JGraphicResourceUserAccess* gUserAccess, const Graphic::J_GRAPHIC_RESOURCE_TYPE rType, const Graphic::J_GRAPHIC_BIND_TYPE bType)
		: gUserAccess(gUserAccess), rType(rType), bType(bType)
	{}
	JGuiImageInfo::JGuiImageInfo(Graphic::ResourceHandle handle)
		: handle(handle), useFirstHandle(true)
	{}

	bool JGuiImageInfo::IsValid()const noexcept
	{
		if (gUserAccess == nullptr && handle == nullptr)
			return false;

		if (handle != nullptr)
			return true;
		else if (useFirstHandle)
			return gUserAccess->GraphicResourceUserInterface().HasFirstHandle();
		else if (displayAllType)
			return gUserAccess->GraphicResourceUserInterface().HasHandle(rType);
		else
			return gUserAccess->GraphicResourceUserInterface().IsValidHandle(rType, dataIndex);
	}
}