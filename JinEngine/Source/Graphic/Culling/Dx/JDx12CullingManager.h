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
#include"../JCullingManager.h"  
#include<d3d12.h>
#include<wrl/client.h>

namespace JinEngine
{
	namespace Graphic
	{ 
		class JCullingUserInterface;
		class JDx12CullingResourceHolder;
		class JDx12GraphicResourceHolder;
		class JDx12CullingManager final : public JCullingManager
		{
		private:
			REGISTER_CLASS_ONLY_USE_TYPEINFO(JDx12CullingManager)
		private:
			std::vector<Microsoft::WRL::ComPtr<ID3D12QueryHeap>> occQueryHeap;
		public:
			~JDx12CullingManager();
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
			JDx12CullingResourceHolder* GetDxHolder(const J_CULLING_TYPE type, int index)const noexcept;
			ID3D12Resource* GetResource(const J_CULLING_TYPE type, int index)const noexcept;
			ID3D12QueryHeap* GetQueryHeap(const uint index)const noexcept;
		private:
			bool HasDependency(const JGraphicInfo::TYPE type)const noexcept final;
			bool HasDependency(const JGraphicOption::TYPE type)const noexcept final;
		private:
			void NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)final;
			void NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)final;
		public:
			void UpdateFrameResourceIndex(const uint frameIndex);
			//for hd occ ... 분할 업데이트를 수행하기때문에 매 프레임마다 update된 영역을 cpu로 복사
			void CopyCullingResultGpuToCpu(JGraphicDevice* device, const uint frameIndex);
		public:
			void StuffClearValue(JGraphicDevice* device, const J_CULLING_TYPE type) final;
		private:
			void ReBuildBuffer(JCullingInfo* info, JGraphicDevice* device, const size_t capacity, const uint index) final;
		public:
			JUserPtr<JCullingInfo> CreateFrsutumData(JGraphicDevice* device, const JCullingCreationDesc& desc) final;
			JUserPtr<JCullingInfo> CreateHzbOcclusionData(JGraphicDevice* device, const JCullingCreationDesc& desc)final;
			JUserPtr<JCullingInfo> CreateHdOcclusionData(JGraphicDevice* device, const JCullingCreationDesc& desc) final;
		private:
			void PostDestruction(const J_CULLING_TYPE type, const uint index) final;
		public:
			bool TryStreamOutCullingBuffer(JCullingInfo* info, const std::string& logName)final;
		public:
			void Clear() final;
		private:
			void ClearResource();
		};

		class JDx12CullingResourceComputeSet
		{
		public:
			JDx12CullingManager* cm = nullptr;
			JCullingInfo* info = nullptr;
			JDx12CullingResourceHolder* cHolder = nullptr;
			JDx12GraphicResourceHolder* gHolder = nullptr;
			ID3D12Resource* resource = nullptr;
		public:
			JDx12CullingResourceComputeSet() = default;
			JDx12CullingResourceComputeSet(JDx12CullingManager* cm, JCullingInfo* info);
			JDx12CullingResourceComputeSet(JDx12CullingManager* cm, const JUserPtr<JCullingInfo>&info);
			JDx12CullingResourceComputeSet(JDx12CullingManager* cm, const JCullingUserInterface& cInterface, const J_CULLING_TYPE cType, const J_CULLING_TARGET cTarget);
		public:
			bool IsValid()const noexcept;
		};
	}
}