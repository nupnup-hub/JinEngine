/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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