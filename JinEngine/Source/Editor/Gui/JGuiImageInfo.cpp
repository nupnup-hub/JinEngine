#include"JGuiImageInfo.h"
#include"../../Graphic/GraphicResource/JGraphicResourceInterface.h"
#include"../../Graphic/GraphicResource/JGraphicResourceUserAccess.h"

namespace JinEngine::Editor
{ 
	JGuiImageInfo::JGuiImageInfo(Graphic::JGraphicResourceUserAccess* gUserAccess, const bool useFirstHandle)
		:gUserAccess(gUserAccess), useFirstHandle(useFirstHandle)
	{}
	JGuiImageInfo::JGuiImageInfo(Graphic::JGraphicResourceUserAccess* gUserAccess,
		const Graphic::J_GRAPHIC_RESOURCE_TYPE rType,
		const bool useFirstHandle)
		: gUserAccess(gUserAccess), rType(rType), useFirstHandle(useFirstHandle)
	{}
	JGuiImageInfo::JGuiImageInfo(Graphic::JGraphicResourceUserAccess* gUserAccess,
		const Graphic::J_GRAPHIC_RESOURCE_TYPE rType,
		const Graphic::J_GRAPHIC_BIND_TYPE bType,
		const bool useFirstHandle)
		: gUserAccess(gUserAccess), rType(rType), bType(bType), useFirstHandle(useFirstHandle)
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