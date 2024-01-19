#pragma once
#include"../JHZBOccCulling.h" 
#include"../../../FrameResource/JOcclusionConstants.h" 
#include"../../../Buffer/Dx/JHlslDebug.h"
#include"../../../../Object/Resource/Shader/JShaderDataHolderBase.h"
#include"../../../../Core/Math/JVector.h"
#include"../../../../Core/Pointer/JOwnerPtr.h"
#include<DirectXCollision.h>

struct ID3D12RootSignature;
struct ID3D12Device;
struct ID3D12GraphicsCommandList;
struct ID3D12Resource;

namespace JinEngine
{
	namespace Graphic
	{  
		class JDx12ComputeShaderDataHolder;
		class JDx12FrameResource;
		class JDx12GraphicResourceManager;
		class JDx12CullingManager;
		class JDx12HZBOccCulling final: public JHZBOccCulling
		{
		public:
			enum class COMPUTE_TYPE
			{
				HZB_COPY_PERSPECTIVE,
				HZB_COPY_ORTHOLOGIC,
				HZB_DOWN_SAMPLING,
				HZB_CULLING_PERSPECTIVE,
				HZB_CULLING_ORTHOLOGIC,
				COUNT
			};
		private:
			std::unique_ptr<JDx12ComputeShaderDataHolder> shader[(uint)COMPUTE_TYPE::COUNT];
		private:
			std::unique_ptr<JDx12GraphicBuffer<JHzbOccDepthMapInfoConstants>> occDepthMapInfoCB;	//static resource
			std::unique_ptr<JDx12GraphicBuffer<uint>> occQueryOutBuffer;
			std::vector<std::unique_ptr<JHlslDebug<JHZBDebugInfo>>> occDebugBuffer;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			bool CanReadBackDebugInfo()const noexcept final;
			bool HasPreprocessing()const noexcept final;
			bool HasPostprocessing()const noexcept final;
		private:
			void NotifyBuildNewHzbOccBuffer(JGraphicDevice* device, const size_t initCapacity, const JUserPtr<JCullingInfo>& cullingInfo) final;
			void NotifyReBuildHzbOccBuffer(JGraphicDevice* device, const size_t capacity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo) final;
			void NotifyDestroyHzbOccBuffer(JCullingInfo* cullingInfo)final;
		private:
			bool BuildOccDebugBuffer(JGraphicDevice* device, const size_t initCapacity, const JUserPtr<JCullingInfo>& cullingInfo);
			void ReBuildOccDebugBuffer(JGraphicDevice* device, const size_t capacity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo);
			void ReBuildObjectConstants(JGraphicDevice* device, const uint objectCapacity);
			void DestroyOccDebugBuffer(JCullingInfo* cullingInfo);
		public:
			//Set upload buffer and shader macro
			void ReBuildOcclusionMapInfoConstants(JGraphicDevice* device, const JGraphicInfo& info) final;	//미적용 occlusion map option 설정하는 인터페이스 추가필요		
		public:
			void StreamOutDebugInfo(const JUserPtr<JCullingInfo>& cullingInfo, const std::wstring& path)final;
			void StreamOutDebugInfo(const std::wstring& path)final;
		public:
			void BeginDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)final;
			void EndDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)final;
		public:
			void DrawOcclusionDepthMap(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper) final;
			void DrawOcclusionDepthMapMultiThread(const JGraphicOccDrawSet* occDrawSet, const JDrawHelper& helper)final;
		public:
			void ComputeOcclusionCulling(const JGraphicHzbOccComputeSet* computeSet, const JDrawHelper& helper) final;
		private:
			void DepthMapDownSampling(ID3D12GraphicsCommandList* commandList,
				JDx12FrameResource* dx12Frame,
				CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapSrvHandle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapSrvHandle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapUavHandle,
				const uint samplingCount,
				const uint srvDescriptorSize,
				const uint passCBIndex,
				const bool isPerspective);
			void OcclusionCulling(ID3D12GraphicsCommandList* commandList,
				JDx12FrameResource* dx12Frame,
				JDx12CullingManager* dx12Cm,
				CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapStHandle,
				const uint passCBIndex,
				const JUserPtr<JCullingInfo>& cullInfo,
				const bool isPerspective);
		private:
			void BuildRootSignature(JGraphicDevice* device, const uint occlusionDsvCapacity);
			void BuildUploadBuffer(JGraphicDevice* device, const uint objectCapacity, const uint occlusionMapCapacity);
		private: 
			void CreateSamplingShader(JGraphicDevice* device, const JGraphicInfo& info, const COMPUTE_TYPE type);
			void CreateCullingShader(JGraphicDevice* device, const uint objectCapacity, const COMPUTE_TYPE type);
		private:
			void CompileShader(JDx12ComputeShaderDataHolder* holder, const JComputeShaderInitData& initData, const COMPUTE_TYPE type);
			void StuffPso(JDx12ComputeShaderDataHolder* holder, JDx12GraphicDevice* dx12Device, const JComputeShaderInitData& initData, const COMPUTE_TYPE type);
		};
	}
} 