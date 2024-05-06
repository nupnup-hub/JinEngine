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


#include"JGraphicResourceManager.h"   
#include"JGraphicResourceUserAccess.h"
#include"JGraphicResourceInterface.h"
#include"../../Window/JWindow.h"
#include"../../Window/JWindowPrivate.h"
namespace JinEngine
{
	namespace Graphic
	{  
		/*
		* create resource senario
		1. Create resource and info(Owner Pointer)
		2. caller(impl) is derived interface(has ginfo and access gmanager public method)
		3. impl interface class can derived user access class(restrict version impl derived interface)
		*/

		void JGraphicResourceManager::ResourceViewInfo::ClearCount()
		{
			count = 0;
		}
		void JGraphicResourceManager::ResourceViewInfo::ClearAllData()
		{
			count = 0;
			capacity = 0;
			offset = 0;
		}
		int JGraphicResourceManager::ResourceViewInfo::GetNextViewIndex()const noexcept
		{
			return offset + count;
		}
		bool JGraphicResourceManager::ResourceViewInfo::HasSpace()const noexcept
		{
			return count < capacity;
		}

		void JGraphicResourceManager::ResourceTypeDesc::ClearCount()
		{
			count = 0;
			for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
				viewInfo[i].ClearCount();
		}
		void JGraphicResourceManager::ResourceTypeDesc::ClearAllData()
		{
			count = 0;
			capacity = 0;
			for (uint i = 0; i < (uint)J_GRAPHIC_BIND_TYPE::COUNT; ++i)
				viewInfo[i].ClearAllData();
		}
		bool JGraphicResourceManager::ResourceTypeDesc::HasSpace()const noexcept
		{
			return count < capacity;
		}
		JGraphicResourceInfo* JGraphicResourceManager::GetInfo(JGraphicResourceUserAccess* access, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_TASK_TYPE task)const noexcept
		{
			if (access == nullptr)
				return nullptr;

			const int arrayIndex = access->GraphicResourceUserInterface().GetResourceArrayIndex(rType, task);
			return arrayIndex != invalidIndex ? GetInfo(rType, arrayIndex) : nullptr;
		}
		HWND JGraphicResourceManager::GetWindowHandle()noexcept
		{
			return Window::JWindowPrivate::HandleInterface::GetHandle();
		}
	}
}

/*
//Debug
 /*
 #include<fstream>
#include"../../Core/File/JFileIOHelper.h"

		std::wofstream stream;
		stream.open(L"D:\\JinWooJung\\gDebug.txt", std::ios::app | std::ios::out);

		Core::JEnumInfo* rInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_RESOURCE_TYPE).name());
		Core::JEnumInfo* bInfo = _JReflectionInfo::Instance().GetEnumInfo(typeid(Graphic::J_GRAPHIC_BIND_TYPE).name());
		auto rNameVec = rInfo->GetEnumNameVec();
		auto bNameVec = bInfo->GetEnumNameVec();
		JFileIOHelper::InputSpace(stream, 1);
 */