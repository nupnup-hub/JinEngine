#pragma once
#include"JGuiData.h"
#include"JGuiConstants.h"
#include"../Device/JGraphicDeviceType.h"
#include"../../Core/JCoreEssential.h"
#include<memory>
#include<vector>
#include<unordered_map>

namespace JinEngine 
{
	namespace Graphic
	{
		class JGraphicDevice;
		class JGraphicResourceManager;
		class JFrameResource;
		class JGuiBackendDataAdaptee;

		//use communication
		//create data for guibackend(update, draw)
		class JGuiBackendDataAdapter
		{ 
		private:  
			using GuiAdapteeMap = std::unordered_map<GuiIdentification, std::unique_ptr<JGuiBackendDataAdaptee>>;
		private:
			GuiAdapteeMap adapteeMap[(uint)J_GRAPHIC_DEVICE_TYPE::COUNT];
		public:
			void AddAdaptee(std::unique_ptr<JGuiBackendDataAdaptee>&& newAdaptee);
		public:
			std::unique_ptr<JGuiInitData> CreateInitData(_In_ JGraphicDevice* device,
				_In_ JGraphicResourceManager* gm,
				_In_ const J_GRAPHIC_DEVICE_TYPE deviceType,
				_In_ const GuiIdentification guiIden);
			std::unique_ptr<JGuiDrawData> CreateDrawData(_In_ JGraphicDevice* device,
				_In_ JGraphicResourceManager* gm,
				_In_ JFrameResource* frame,
				_In_ const J_GRAPHIC_DEVICE_TYPE deviceType,
				_In_ const GuiIdentification guiIden);
		};
	}
}