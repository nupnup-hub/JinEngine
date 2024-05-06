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
#include"../JGpuAcceleratorManager.h"
#include"../../FrameResource/JFrameIndexAccess.h"
#include<d3d12.h>
#include<wrl/client.h>

namespace JinEngine
{
	namespace Graphic
	{
		class JGpuAcceleratorHolder;
		class JDx12GpuAcceleratorHolder;
		class JGpuAcceleratorUserInterface;

		class JDx12GpuAcceleratorManager : public JGpuAcceleratorManager
		{ 
		public:
			struct BuildData;
		private:
			JFrameIndexAccess* frameAccess = nullptr;
			std::vector<JOwnerPtr<JGpuAcceleratorInfo>> infoVec;
		public:
			JDx12GpuAcceleratorManager(JFrameIndexAccess* frameAccess);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			JGpuAcceleratorInfo* GetInfo(const uint index)const noexcept final;
			JDx12GpuAcceleratorHolder* GetDx12Holder(JGpuAcceleratorInfo* info)const noexcept;
		public: 
			JUserPtr<JGpuAcceleratorInfo> Create(JGraphicDevice* device, JGraphicResourceManager* gm, const JGpuAcceleratorBuildDesc& desc) final;
			bool Destroy(JGraphicDevice* device, JGraphicResourceManager* gm, JGpuAcceleratorInfo* info) final;
		public: 
			void UpdateTransform(JGraphicDevice* device, JGraphicResourceManager* gm, JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp) final;
			void Add(JGraphicDevice* device, JGraphicResourceManager* gm, JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp) final;
			void Remove(JGraphicDevice* device, JGraphicResourceManager* gm, JGpuAcceleratorInfo* info, const JUserPtr<JComponent>& comp) final;
		private:
			JOwnerPtr<JGpuAcceleratorInfo> CreateInfo(std::unique_ptr<JGpuAcceleratorHolder>&& holder, const JGpuAcceleratorBuildDesc& desc);
		private:
			std::unique_ptr<JDx12GpuAcceleratorHolder> BuildAcceleratorStructure(BuildData& buildData);
			void UpdateInstance(const BuildData& buildData, JDx12GpuAcceleratorHolder* holder, const J_GPU_ACCELERATOR_BUILD_OPTION preBuildOption);
			void AddBottomLevelAs(BuildData& buildData, JDx12GpuAcceleratorHolder* holder);
			void RemoveBottomLevelAs(const BuildData& buildData, JDx12GpuAcceleratorHolder* holder);
		};


		struct JDx12AcceleratorResourceComputeSet
		{ 
		public:
			JGpuAcceleratorInfo* info;
			JDx12GpuAcceleratorManager* am = nullptr;
			JDx12GpuAcceleratorHolder* holder = nullptr;   
		public:
			JDx12AcceleratorResourceComputeSet() = default;
			JDx12AcceleratorResourceComputeSet(JDx12GpuAcceleratorManager* am, const JUserPtr<JGpuAcceleratorInfo>& aInfo);
			JDx12AcceleratorResourceComputeSet(JDx12GpuAcceleratorManager* am, const JGpuAcceleratorUserInterface& user);
		public:
			bool IsValid()const noexcept;
		};
	}
}