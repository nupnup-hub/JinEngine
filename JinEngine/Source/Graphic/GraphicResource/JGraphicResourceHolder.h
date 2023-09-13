#pragma once
#include"../Device/JGraphicDeviceType.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../../Core/JCoreEssential.h" 
#include<string>

struct ID3D12Resource;
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
		public:
			virtual bool HasValidResource()const noexcept = 0;
		public:
			virtual void Clear() = 0;
		};
	}
}