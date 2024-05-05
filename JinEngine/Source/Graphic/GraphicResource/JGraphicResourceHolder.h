#pragma once 
#include"JGraphicResourceType.h"
#include"../Device/JGraphicDeviceUser.h" 
#include"../../Core/Math/JVector.h" 
 
namespace JinEngine
{
	namespace Graphic
	{
		class JGraphicResourceHolder : public JGraphicDeviceUser
		{
		public:
			virtual ~JGraphicResourceHolder() = default;
		public: 
			virtual uint GetWidth()const noexcept = 0;
			virtual uint GetHeight()const noexcept = 0;
			virtual uint GetArrayCount()const noexcept = 0;
			virtual uint GetElementCount()const noexcept = 0;
			virtual uint GetElementSize()const noexcept = 0; 
			virtual J_GRAPHIC_RESOURCE_FORMAT GetEngineFormat()const noexcept = 0;
			virtual JVector2<uint> GetResourceSize()const noexcept = 0;
		public: 
			virtual bool HasValidResource()const noexcept = 0;
		public:
			virtual void Clear() = 0;
		};
	}
}