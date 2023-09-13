#include"JGraphicResourceManager.h"   
#include"../../Window/JWindow.h"
#include"../../Window/JWindowPrivate.h"
namespace JinEngine
{
	namespace Graphic
	{
		namespace Private
		{
			static constexpr uint occlusionMipmapViewCapacity = 10;
			static constexpr uint minOcclusionSize = 8;
		}
		 
		/*
		* create resource senario
		1. Create resource and info(Owner Pointer)
		2. caller(impl) is derived interface(has ginfo and access gmanager public method)
		3. impl interface class can derived user access class(restrict version impl derived interface)
		*/
		uint JGraphicResourceManager::GetOcclusionMipMapViewCapacity()noexcept
		{
			return Private::occlusionMipmapViewCapacity;
		}
		uint JGraphicResourceManager::GetOcclusionMinSize()noexcept
		{
			return Private::minOcclusionSize;
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