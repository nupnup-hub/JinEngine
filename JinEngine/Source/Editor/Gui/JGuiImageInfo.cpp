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
//#include"../../Graphic/GraphicResource/JGraphicResourceInterface.h" 

namespace JinEngine::Editor
{ 
	JGuiImageInfo::JGuiImageInfo(JGraphicResourceUserInterface* gUser)
		:gUser(gUser), useFirstHandle(true)
	{} 
	JGuiImageInfo::JGuiImageInfo(JGraphicResourceUserInterface* gUser, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)
		: gUser(gUser), rType(rType), bType(bType)
	{}
	JGuiImageInfo::JGuiImageInfo(JComponent* comp)
		:gUser(comp->ModuleManagedData()->GetGraphicResourceUserInterface()), useFirstHandle(true)
	{}
	JGuiImageInfo::JGuiImageInfo(JComponent* comp, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)
		: gUser(comp->ModuleManagedData()->GetGraphicResourceUserInterface()), rType(rType), bType(bType)
	{}
	JGuiImageInfo::JGuiImageInfo(JResourceObject* resource)
		: gUser(resource->ModuleManagedData()->GetGraphicResourceUserInterface()), useFirstHandle(true)
	{}
	JGuiImageInfo::JGuiImageInfo(JResourceObject* resource, const J_GRAPHIC_RESOURCE_TYPE rType, const J_GRAPHIC_BIND_TYPE bType)
		: gUser(resource->ModuleManagedData()->GetGraphicResourceUserInterface()), rType(rType), bType(bType)
	{}
	JGuiImageInfo::JGuiImageInfo(ResourceHandle handle)
		: handle(handle), useFirstHandle(true)
	{}

	bool JGuiImageInfo::IsValid()const noexcept
	{
		if (gUser == nullptr && handle == nullptr)
			return false;

		if (handle != nullptr)
			return true;
		else if (useFirstHandle)
			return gUser->HasFirstHandle();
		else if (displayAllType)
			return gUser->GetResourceCount(rType) > 0;
		else
			return gUser->IsValidHandle(rType, dataIndex);
	}
}