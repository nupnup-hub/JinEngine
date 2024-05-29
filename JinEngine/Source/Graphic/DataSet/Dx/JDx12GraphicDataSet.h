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
#include"../JGraphicDataSet.h"
#include<d3dx12.h>
 
namespace JinEngine
{
	namespace Graphic
	{ 
		class JCommandContext;
		class JGraphicResourceInfo;
		struct JDx12GraphicBindSet final : public JGraphicBindSet
		{ 
		public:
			JDx12GraphicBindSet(JCommandContext* context);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
 
		struct JDx12GraphicOccDrawSet;
		struct JDx12GraphicDepthMapDrawSet final : public JGraphicDepthMapDrawSet
		{ 
		public:
			JDx12GraphicDepthMapDrawSet(JCommandContext* context);
			JDx12GraphicDepthMapDrawSet(const JDx12GraphicOccDrawSet* drawSet);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicDebugRsComputeSet final : public JGraphicDebugRsComputeSet
		{ 
		public: 
			JDx12GraphicDebugRsComputeSet(JCommandContext* context);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
		 
		struct JDx12GraphicSceneDrawSet final : public JGraphicSceneDrawSet
		{ 
		public:
			JDx12GraphicSceneDrawSet(JCommandContext* context);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicShadowMapDrawSet final : public JGraphicShadowMapDrawSet
		{ 
		public:
			JDx12GraphicShadowMapDrawSet(JCommandContext* context, JBlur* blur);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicOccDrawSet final : public JGraphicOccDrawSet
		{ 
		public:
			JDx12GraphicOccDrawSet(JCommandContext* context, JDepthTest* depthTest);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
 
		struct JDx12GraphicHzbOccComputeSet final : public JGraphicHzbOccComputeSet
		{ 
		public:
			JDx12GraphicHzbOccComputeSet(JCommandContext* context);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicHdOccExtractSet final : public JGraphicHdOccExtractSet
		{ 
		public:
			JDx12GraphicHdOccExtractSet(JCommandContext* context);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
 
		struct JDx12GraphicOutlineDrawSet : public JGraphicOutlineDrawSet
		{
		public: 
			CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE stencilMapHandle;
		public:
			const bool useHandle = false;
		public:
			JDx12GraphicOutlineDrawSet(JCommandContext* context);
			JDx12GraphicOutlineDrawSet(JCommandContext* context, CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle, CD3DX12_GPU_DESCRIPTOR_HANDLE stencilMapHandle);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicBlurComputeSet : public JGraphicBlurComputeSet
		{
		public:
			JGraphicResourceInfo* srcInfo = nullptr;		//option if has info try to transpose state
			JGraphicResourceInfo* destInfo = nullptr;		//option if has info try to transpose state
		public: 
			CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle;
		public:
			JDx12GraphicBlurComputeSet(JCommandContext* context,
				std::unique_ptr<JBlurDesc>&& desc,
				CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicDownSampleComputeSet : public JGraphicDownSampleComputeSet
		{
		public:
			JGraphicResourceInfo* info = nullptr;		//option if has info try to transpose state
		public: 
			//mipmap handle은 순차적으로 배열되있어여한다.
			CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle;
		public:
			JDx12GraphicDownSampleComputeSet(JCommandContext* context, 
				JBlur* blur,
				std::unique_ptr<JDownSampleDesc>&& desc,
				const std::vector<Core::JDataHandle>& handle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicSsaoComputeSet : public JGraphicSsaoComputeSet
		{ 
		public:
			JDx12GraphicSsaoComputeSet(JCommandContext* context, JGraphicResourceShareData* shareData);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicAAComputeSet : public JGraphicAAComputeSet
		{ 
		public:
			JDx12GraphicAAComputeSet(JCommandContext* context, JGraphicResourceShareData* shareData);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicConvertColorComputeSet : public JGraphicConvertColorComputeSet
		{
		public:
			JGraphicResourceInfo* srcInfo = nullptr;		//option if has info try to transpose state
			JGraphicResourceInfo* destInfo = nullptr;		//option if has info try to transpose state
		public:
			JDx12GraphicConvertColorComputeSet(JCommandContext* context, const JConvertColorDesc& desc, JGraphicResourceInfo* srcInfo, JGraphicResourceInfo* destInfo);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12PostProcessComputeSet : public JPostProcessComputeSet
		{ 
		public:
			JDx12PostProcessComputeSet(JPostProcessEffectSet* ppSet, JCommandContext* context, JGraphicResourceManager* gm, JGraphicResourceShareData* shareData);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;

		};

		struct JDx12GraphicLightCullingTaskSet : public JGraphicLightCullingTaskSet
		{ 
		public:
			JDx12GraphicLightCullingTaskSet(JCommandContext* context);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicLightCullingDebugDrawSet : public JGraphicLightCullingDebugDrawSet
		{ 
		public:
			JDx12GraphicLightCullingDebugDrawSet(JCommandContext* context);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicRtAoComputeSet : public JGraphicRtAoComputeSet
		{ 
		public:
			JDx12GraphicRtAoComputeSet(JCommandContext* context);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicRtGiComputeSet : public JGraphicRtGiComputeSet
		{
		public:
			JGraphicDevice* device;
			JGraphicResourceShareData* shareData;
		public:
			JDx12GraphicRtGiComputeSet(JCommandContext* context, JGraphicDevice* device, JGraphicResourceShareData* shareData);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
		 
		struct JDx12GraphicRtDenoiseComputeSet : public JGraphicRtDenoiseComputeSet
		{
		public:
			JGraphicDevice* device;
			JGraphicResourceManager* gm;
			JGraphicResourceShareData* shareData;
		public:
			JDx12GraphicRtDenoiseComputeSet(JCommandContext* context, JGraphicDevice* device, JGraphicResourceManager* gm, JGraphicResourceShareData* shareData);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicInfoChangedSet : public JGraphicInfoChangedSet
		{
		public:
			JGraphicDevice* device;
			JGraphicResourceManager* gm;
			JCullingManager* cm;
		public:
			JDx12GraphicInfoChangedSet(const JGraphicInfo& preInfo,
				const JGraphicInfo& newInfo,
				JGraphicDevice* device,
				JGraphicResourceManager* gm,
				JCullingManager* cm);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
		struct JDx12GraphicOptionChangedSet : public JGraphicOptionChangedSet
		{
		public:
			JGraphicDevice* device;
			JGraphicResourceManager* gm;
		public:
			JDx12GraphicOptionChangedSet(const JGraphicOption& preOption,
				const JGraphicOption& newOption, 
				JGraphicDevice* device,
				JGraphicResourceManager* gm);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
	}
}