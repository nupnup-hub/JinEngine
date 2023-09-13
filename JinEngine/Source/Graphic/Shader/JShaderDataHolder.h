#pragma once
#include"../Device/JGraphicDeviceUser.h"
#include"../../Object/Resource/Shader/JShaderDataHolderBase.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicShaderDataHolder : public JGraphicShaderDataHolderBase, public JGraphicDeviceUser
		{
		public:
			JGraphicShaderDataHolder() = default;
		public:
			DeviceIdentification GetDeviceIdentification()const noexcept;
		public: 
			bool IsSameDevice(const DeviceIdentification otherDeviceIdentification)const noexcept final;
		};

		class JComputeShaderDataHolder : public JComputeShaderDataHolderBase, public JGraphicDeviceUser
		{
		public:
			JComputeShaderDataHolder() = default;
		public:
			DeviceIdentification GetDeviceIdentification()const noexcept;
		public:
			bool IsSameDevice(const DeviceIdentification otherDeviceIdentification)const noexcept final;
		};
	}
}