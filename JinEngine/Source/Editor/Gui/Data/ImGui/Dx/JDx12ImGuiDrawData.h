#pragma once
#include"../JImGuiDrawData.h"

struct ID3D12GraphicsCommandList;
namespace JinEngine
{
	namespace Editor
	{
		struct JDx12ImGuiDrawData final : public JImGuiDrawData
		{
		public:
			ID3D12GraphicsCommandList* cmd;
		public:
			JDx12ImGuiDrawData(ID3D12GraphicsCommandList* cmd);
		public:
			Graphic::J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
	}
}