#pragma once 
#include"JGpuAcceleratorType.h"
#include"../JGraphicSubClassInterface.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../DataSet/JGraphicDataSet.h" 
#include"../../Core/Math/JVector.h"
#include"../../Core/Reflection/JReflection.h"  

namespace JinEngine
{  
	class JComponent;
	class JGameObject;
	namespace Graphic
	{  
		class JGraphicDevice;
		class JGraphicResourceManager;
		class JGpuAcceleratorInfo;
		class JGpuAcceleratorHolder;

		class JGpuAcceleratorManager : public JGraphicDeviceUser, public JGraphicSubClassInterface
		{
		public:
			JGpuAcceleratorHolder* GetHolder(JGpuAcceleratorInfo* info)const noexcept;
			virtual JGpuAcceleratorInfo* GetInfo(const uint index)const noexcept = 0;
		public: 
			virtual JUserPtr<JGpuAcceleratorInfo> Create(JGraphicDevice* device, JGraphicResourceManager* gm, const JGpuAcceleratorBuildDesc& desc) = 0;
			virtual bool Destroy(JGraphicDevice* device, JGraphicResourceManager* gm, JGpuAcceleratorInfo* info) = 0;
		public:
			virtual void UpdateTransform(JGraphicDevice* device, JGraphicResourceManager* gm, JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp) = 0;
			virtual void Add(JGraphicDevice* device, JGraphicResourceManager* gm, JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp) = 0;
			virtual void Remove(JGraphicDevice* device, JGraphicResourceManager* gm, JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp) = 0;
		};
	}
}