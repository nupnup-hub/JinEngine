#pragma once
#include"../JHZBOccCulling.h" 
#include"../../../FrameResource/JOcclusionConstants.h" 
#include"../../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../../Culling/Dx/JDx12CullingManager.h"
#include"../../../Buffer/Dx/JHlslDebug.h" 
#include"../../../Shader/Dx/JDx12ShaderDataHolder.h"
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
		class JDx12FrameResource;  
		class JDx12CommandContext;
		 
		class JDx12HZBOccCulling final: public JHZBOccCulling
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
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
			std::unique_ptr<JDx12GraphicBufferT<JHzbOccDepthMapInfoConstants>> occDepthMapInfoCB;	//static resource
			std::unique_ptr<JDx12GraphicBufferT<uint>> occQueryOutBuffer;
			std::vector<std::unique_ptr<JHlslDebug<JHZBDebugInfo>>> occDebugBuffer;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
		public:
			~JDx12HZBOccCulling();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			bool CanReadBackDebugInfo()const noexcept final;
			bool HasPreprocessing()const noexcept final;
			bool HasPostprocessing()const noexcept final;
		private:
			bool HasDependency(const JGraphicInfo::TYPE type)const noexcept final;
			bool HasDependency(const JGraphicOption::TYPE type)const noexcept final;
		private:
			void NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)final;
			void NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)final;
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
			void DepthMapDownSampling(JDx12CommandContext* context,
				CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapSrvHandle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapSrvHandle,
				CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapUavHandle,
				const uint samplingCount, 
				const uint passCBIndex,
				const bool isPerspective);
			void OcclusionCulling(JDx12CommandContext* context,
				CD3DX12_GPU_DESCRIPTOR_HANDLE mipMapStHandle,
				const uint passCBIndex,
				const JDx12CullingResourceComputeSet& cSet,
				const bool isPerspective);
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info);
			void BuildRootSignature(JGraphicDevice* device, const uint occlusionDsvCapacity);
			void BuildUploadBuffer(JGraphicDevice* device, const uint objectCapacity, const uint occlusionMapCapacity);
		private:
			void ClearResource();
		private: 
			void CreateSamplingShader(JGraphicDevice* device, const JGraphicInfo& info, const COMPUTE_TYPE type);
			void CreateCullingShader(JGraphicDevice* device, const uint objectCapacity, const COMPUTE_TYPE type);
		private:
			void CompileShader(JDx12ComputeShaderDataHolder* holder, const JComputeShaderInitData& initData, const COMPUTE_TYPE type);
		};
	}
} 