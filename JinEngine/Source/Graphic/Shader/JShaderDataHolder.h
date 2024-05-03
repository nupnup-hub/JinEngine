#pragma once
#include"JShaderType.h"
#include"../Device/JGraphicDeviceUser.h" 
#include"../JGraphicConstants.h"
#include"../../Core/Platform/JHardwareInfo.h"
 
namespace JinEngine
{
	namespace Graphic
	{
		class JShaderDataHolder : public JGraphicDeviceUser
		{ 
		public:
			JShaderDataHolder();
			~JShaderDataHolder();
		public:
			virtual void Clear() = 0;
		public:
			virtual J_SHADER_TYPE GetShaderType()const noexcept = 0;
			virtual uint GetVariationCount()const noexcept = 0;
			virtual ResourceHandle GetShaderData(const uint index)const noexcept = 0; 
		}; 

		class JGrahicShaderDataHolder : public JShaderDataHolder
		{ 
		};
		class JComputeShaderDataHolder : public JShaderDataHolder
		{
		public:
			JDispatchInfo dispatchInfo;
		};
	}
}