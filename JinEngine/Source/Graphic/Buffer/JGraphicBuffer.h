#pragma once 
#include"JBufferType.h" 
#include"../Device/JGraphicDevice.h" 
#include"../../Core/JCoreEssential.h"
#include"../../Core/Exception/JExceptionMacro.h"
#include"../../Core/Interface/JValidInterface.h" 

namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicDevice;
		class JGraphicBufferBase : public JGraphicDeviceUser, public Core::JValidInterface
		{
		public:
			virtual ~JGraphicBufferBase() = default;
		public:
			virtual void Build(JGraphicDevice* device, const uint elementcount) = 0;
			virtual void Clear()noexcept = 0;
		public:
			virtual void CopyData(const uint elementIndex, const void* data) = 0;
			virtual void CopyData(const uint elementIndex, const uint count, const void* data) = 0;
		public:
			virtual J_GRAPHIC_BUFFER_TYPE GetBufferType()const noexcept = 0;
			virtual uint GetElementCount()const noexcept = 0;
			virtual uint GetElementByteSize()const noexcept = 0;
			virtual BYTE* GetCpuBytePointer()const noexcept = 0;
		public:
			virtual bool CanMappedCpuPointer()const noexcept = 0;
		};
	}
}