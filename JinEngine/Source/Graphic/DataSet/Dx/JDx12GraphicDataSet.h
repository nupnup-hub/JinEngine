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

		struct JDx12GraphicDepthMapDebugObjectSet final : public JGraphicDepthMapDebugObjectSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList; 
		public:
			JDx12GraphicDepthMapDebugObjectSet(JGraphicResourceManager* graphicResourceM, ID3D12GraphicsCommandList* cmdList);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicDepthMapDebugHandleSet final : public JGraphicDepthMapDebugHandleSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;
		public:
			CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle;
		public: 
			JDx12GraphicDepthMapDebugHandleSet(const JVector2<uint> size, 
				const float nearF,
				const float farF, 
				ID3D12GraphicsCommandList* cmdList,
				CD3DX12_GPU_DESCRIPTOR_HANDLE srcHandle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE destHandle);
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

		struct JDx12GraphicOccDebugDrawSet final : public JGraphicOccDebugDrawSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;
		public:
			JDx12GraphicOccDebugDrawSet(JGraphicDevice* device,
				JGraphicResourceManager* graphicResourceM,
				JDepthMapDebug* depthDebug,
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

		struct JDx12GraphicOutlineObjectSet : public JGraphicOutlineObjectSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;
		public:
			JDx12GraphicOutlineObjectSet(JGraphicDevice* device,
				JGraphicResourceManager* graphicResourceM,
				ID3D12GraphicsCommandList* cmdList);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};

		struct JDx12GraphicOutlineHandleSet : public JGraphicOutlineHandleSet
		{
		public:
			ID3D12GraphicsCommandList* cmdList;
			CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle;
			CD3DX12_GPU_DESCRIPTOR_HANDLE stencilMapHandle;
		public:
			JDx12GraphicOutlineHandleSet(JGraphicResourceManager* graphicResourceM, 
				ID3D12GraphicsCommandList* cmdList,
			CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapHandle,
			CD3DX12_GPU_DESCRIPTOR_HANDLE stencilMapHandle);
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		};
	}
}