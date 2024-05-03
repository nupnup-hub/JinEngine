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