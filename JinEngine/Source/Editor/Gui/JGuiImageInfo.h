#pragma once 
#include"../../Graphic/GraphicResource/JGraphicResourceType.h" 

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
			Graphic::JGraphicResourceUserAccess* gUserAccess;
			ExtraFuncPerImagePtr extraPerImagePtr = nullptr;	//excute after display valid image
		public:
			Graphic::J_GRAPHIC_RESOURCE_TYPE rType;
			Graphic::J_GRAPHIC_BIND_TYPE bType = Graphic::J_GRAPHIC_BIND_TYPE::SRV;
			uint dataIndex = 0;
			uint bIndex = 0;
		public:
			bool displayAllType = true;
			bool useFirstHandle = true;
		public:
			JGuiImageInfo(Graphic::JGraphicResourceUserAccess* gUserAccess, const bool useFirstHandle = true);
			JGuiImageInfo(Graphic::JGraphicResourceUserAccess* gUserAccess,
				const Graphic::J_GRAPHIC_RESOURCE_TYPE rType,
				const bool useFirstHandle = false);
			JGuiImageInfo(Graphic::JGraphicResourceUserAccess* gUserAccess,
				const Graphic::J_GRAPHIC_RESOURCE_TYPE rType,
				const Graphic::J_GRAPHIC_BIND_TYPE bType,
				const bool useFirstHandle = false);
		public:
			bool IsValid()const noexcept;
		};
	}
}

