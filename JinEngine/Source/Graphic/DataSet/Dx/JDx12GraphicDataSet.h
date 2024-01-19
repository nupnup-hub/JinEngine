#pragma once
#include"../JGraphicDataSet.h"
#include<d3dx12.h>
 
namespace JinEngine
{
	namespace Graphic
	{ 
		struct JDx12GraphicBindSet final : public JGraphicBindSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;
		public:
			JDx12GraphicBindSet(JFrameResource* currFrame,
				JGraphicResourceManager* graphicResourceM,
				JCullingManager* cullingManager,
				ID3D12GraphicsCommandList* cmdList);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
 
		struct JDx12GraphicOccDrawSet;
		struct JDx12GraphicDepthMapDrawSet final : public JGraphicDepthMapDrawSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;
		public:
			JDx12GraphicDepthMapDrawSet(JFrameResource* currFrame,
				JGraphicResourceManager* graphicResourceM,
				JCullingManager* cullingM,
				ID3D12GraphicsCommandList* cmdList);
			JDx12GraphicDepthMapDrawSet(const JDx12GraphicOccDrawSet* drawSet);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicDebugRsComputeSet final : public JGraphicDebugRsComputeSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList; 
		public: 
			JDx12GraphicDebugRsComputeSet(JGraphicDevice* gDevice, JGraphicResourceManager* graphicResourceM, ID3D12GraphicsCommandList* cmdList);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
		 
		struct JDx12GraphicSceneDrawSet final : public JGraphicSceneDrawSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;
		public:
			JDx12GraphicSceneDrawSet(JGraphicDevice* device,
				JFrameResource* currFrame,
				JGraphicResourceManager* graphicResourceM,
				JCullingManager* cullingM,
				ID3D12GraphicsCommandList* cmdList);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicShadowMapDrawSet final : public JGraphicShadowMapDrawSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;
		public:
			JDx12GraphicShadowMapDrawSet(JGraphicDevice* device,
				JFrameResource* currFrame,
				JGraphicResourceManager* graphicResourceM,
				JCullingManager* cullingM,
				JImageProcessing* image,
				ID3D12GraphicsCommandList* cmdList);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicOccDrawSet final : public JGraphicOccDrawSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;
		public:
			JDx12GraphicOccDrawSet(JGraphicDevice* device,
				JFrameResource* currFrame,
				JGraphicResourceManager* graphicResourceM,
				JCullingManager* cullingM,
				JDepthTest* depthTest,
				ID3D12GraphicsCommandList* cmdList);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
 
		struct JDx12GraphicHzbOccComputeSet final : public JGraphicHzbOccComputeSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;
		public:
			JDx12GraphicHzbOccComputeSet(JFrameResource* currFrame,
				JGraphicResourceManager* graphicResourceM,
				JCullingManager* cullingM,
				ID3D12GraphicsCommandList* cmdList);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicHdOccExtractSet final : public JGraphicHdOccExtractSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;
		public:
			JDx12GraphicHdOccExtractSet(JCullingManager* cullingM, ID3D12GraphicsCommandList* cmdList);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
 
		struct JDx12GraphicOutlineDrawSet : public JGraphicOutlineDrawSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;
			CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE stencilMapHandle;
		public:
			const bool useHandle = false;
		public:
			JDx12GraphicOutlineDrawSet(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM, ID3D12GraphicsCommandList* cmdList);
			JDx12GraphicOutlineDrawSet(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM,
				ID3D12GraphicsCommandList* cmdList,
				CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE stencilMapHandle);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicBlurComputeSet : public JGraphicBlurComputeSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;
			CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle;
		public:
			JDx12GraphicBlurComputeSet(JGraphicDevice* device,
				std::unique_ptr<JBlurDesc>&& desc,
				ID3D12GraphicsCommandList* cmdList,
				CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicDownSampleComputeSet : public JGraphicDownSampleComputeSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;
			//mipmap handle은 순차적으로 배열되있어여한다.
			CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle;
		public:
			JDx12GraphicDownSampleComputeSet(JGraphicDevice* device,
				JGraphicResourceManager* graphicResourceM,
				std::unique_ptr<JDownSampleDesc>&& desc,
				std::vector<Core::JDataHandle>&& handle,
				ID3D12GraphicsCommandList* cmdList,
				CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicSsaoComputeSet : public JGraphicSsaoComputeSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;  
		public:
			JDx12GraphicSsaoComputeSet(JGraphicDevice* device,
				JGraphicResourceManager* graphicResourceM,
				JFrameResource* currFrame, 
				ID3D12GraphicsCommandList* cmdList);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicLightCullingTaskSet : public JGraphicLightCullingTaskSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;
		public:
			JDx12GraphicLightCullingTaskSet(JGraphicDevice* device,
				JGraphicResourceManager* graphicResourceM,
				JCullingManager* cullingManager,
				JFrameResource* currFrame,
				ID3D12GraphicsCommandList* cmdList);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicLightCullingDebugDrawSet : public JGraphicLightCullingDebugDrawSet
		{
		public: 
			ID3D12GraphicsCommandList* cmdList;
		public:
			JDx12GraphicLightCullingDebugDrawSet(JGraphicDevice* device,
				JGraphicResourceManager* graphicResourceM,
				ID3D12GraphicsCommandList* cmdList);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
	}
}