#pragma once
#include"../JImageProcessing.h" 
#include"../../Buffer/Dx/JDx12GraphicBuffer.h"
#include"../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include"../../../Core/Math/JMatrix.h"
#include<wrl/client.h> 
#include<dxgiformat.h>

namespace JinEngine
{
	class JTexture;
	namespace Graphic
	{
		class JDx12ComputeShaderDataHolder;
		class JDx12GraphicShaderDataHolder;
		class JDx12GraphicResourceManager;
		class JDx12GraphicDevice;
		class JDx12GraphicResourceManager;
		class JDx12FrameResource;
		class JGraphicResourceInfo;

		enum class J_SSAO_FUNCTION
		{ 
			NONE = 0,
			BLUR,
			COUNT
		};
		enum class J_SSAO_BLUR_SHADER
		{
			BILATERAL_X,
			BILATERAL_Y,
			COUNT
		};
		class JDx12ImageProcessing : public JImageProcessing
		{
		public:
			static constexpr uint ssaoMaxSampleCount = 32;
			static constexpr uint ssaoSliceCount = 16;
		private: 
			struct BlurConstants
			{
				JVector2F size;								//8
				JVector2F invSize;							//16
				JMatrix4x2 kernel[JKenelType::MaxSize()];	//240	 
				int mipLevel;								//244
				float blurPad00;							//248
				float blurPad01;							//252
				float blurPad02;							//256
			};
			struct DownSampleConstants
			{ 
				JMatrix4x2 kernel[JKenelType::MaxSize()];			//224 
			};
			struct SsaoSampleConstants
			{
				JVector4F sample[ssaoMaxSampleCount];
			};
			struct SsaoAoSliceConstants
			{
				JVector4F jitter;
				JVector2F posOffset;
				float sliceIndex; 
				uint uSliceIndex;
			};
		private:
			struct SsaoIntermediate
			{
			public:
				JUserPtr<JGraphicResourceInfo> aoIntermediate00;
				JUserPtr<JGraphicResourceInfo> aoIntermediate01;
				JUserPtr<JGraphicResourceInfo> aoInterleave;  
				JUserPtr<JGraphicResourceInfo> depth;
				JUserPtr<JGraphicResourceInfo> depthInterleave;
			public:
				int sameResolutionInfoCount = 0;
			public:
				SsaoIntermediate(JGraphicDevice* device, JGraphicResourceManager* gM, const uint width, const uint height);
				~SsaoIntermediate();
				SsaoIntermediate(SsaoIntermediate&& rhs) = default;
				SsaoIntermediate& operator=(SsaoIntermediate&& rhs) = default;
			}; 
			struct SSaoDrawDataSet
			{
			public:
				JDx12GraphicDevice* dx12Device;
				JDx12GraphicResourceManager* dx12Gm;
				JDx12FrameResource* dx12Frame;
				ID3D12GraphicsCommandList* cmdList;
			public:
				JSsaoDesc ssaoDesc;
				bool canBlur;
				bool canUseHbaoInterleave = false;
				bool isValid = false; 
			public:
				SsaoIntermediate* inter = nullptr;
				int depthInterleaveRtvHeapSt = 0;
				int depthInterleaveSrvHeapSt = 0;
			public: 
				CD3DX12_GPU_DESCRIPTOR_HANDLE depthSrvHandle;
				CD3DX12_GPU_DESCRIPTOR_HANDLE normalSrvHandle;
				CD3DX12_GPU_DESCRIPTOR_HANDLE randomSrvHandle;
				CD3DX12_CPU_DESCRIPTOR_HANDLE aoRtvHandle;
				CD3DX12_CPU_DESCRIPTOR_HANDLE aoIntermediate00RtvHandle;
				CD3DX12_GPU_DESCRIPTOR_HANDLE aoIntermediate00SrvHandle;
				CD3DX12_CPU_DESCRIPTOR_HANDLE aoIntermediate01RtvHandle;
				CD3DX12_GPU_DESCRIPTOR_HANDLE aoIntermediate01SrvHandle;
				CD3DX12_CPU_DESCRIPTOR_HANDLE aoInterleaveRtvHandle;
				CD3DX12_GPU_DESCRIPTOR_HANDLE aoInterleaveSrvHandle;
				CD3DX12_CPU_DESCRIPTOR_HANDLE aoDepthRtvHandle;
				CD3DX12_GPU_DESCRIPTOR_HANDLE aoDepthSrvHandle;
				CD3DX12_CPU_DESCRIPTOR_HANDLE aoDepthInterleaveRtvHandle;
				CD3DX12_GPU_DESCRIPTOR_HANDLE aoDepthInterleaveSrvHandle;
			public:
				ID3D12Resource* rtResource;
				ID3D12Resource* aoResource;
				ID3D12Resource* aoInter00Resource;
				ID3D12Resource* aoInter01Resource;
				ID3D12Resource* aoInterleaveResource;
				ID3D12Resource* aoDepthResource;
				ID3D12Resource* aoDepthInterleaveResource; 
			public:
				SSaoDrawDataSet(JGraphicSsaoComputeSet* computeSet,
					const JDrawHelper& helper,
					const std::unordered_map<size_t, std::unique_ptr<SsaoIntermediate>>& ssaoInterMap,
					const JUserPtr<JGraphicResourceInfo>(&randomInfoVec)[(uint)J_SSAO_TYPE::COUNT]);
			};
		private:
			const size_t guid;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> blurRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> downSampleRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> ssaoRootSignature; 
			Microsoft::WRL::ComPtr<ID3D12RootSignature> hbaoRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> ssaoBlurRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> ssaoCombineRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> ssaoDepthLinearizeRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> ssaoDepthInterleaveRootSignature;
		private:
			std::unique_ptr<JDx12ComputeShaderDataHolder> blur[(uint)J_BLUR_TYPE::COUNT][(uint)J_KERNEL_SIZE::COUNT];
			std::unique_ptr<JDx12ComputeShaderDataHolder> downSample[(uint)J_DOWN_SAMPLING_TYPE::COUNT][(uint)J_KERNEL_SIZE::COUNT];
			std::unique_ptr<JDx12GraphicShaderDataHolder> ssao[(uint)J_SSAO_TYPE::COUNT * (uint)J_SSAO_SAMPLE_TYPE::COUNT * (uint)J_SSAO_FUNCTION::COUNT];
			std::unique_ptr<JDx12GraphicShaderDataHolder> ssaoBlur[(uint)J_SSAO_BLUR_SHADER::COUNT][(uint)J_KERNEL_SIZE::COUNT];
			std::unique_ptr<JDx12GraphicShaderDataHolder> ssaoCombine[(uint)J_SSAO_FUNCTION::COUNT];
			std::unique_ptr<JDx12GraphicShaderDataHolder> ssaoDepthLinearize;
			std::unique_ptr<JDx12GraphicShaderDataHolder> ssaoDepthInterleave;
		private:
			//inner buffer
			std::unique_ptr<JDx12GraphicBuffer<BlurConstants>> blurCB;
			std::unique_ptr<JDx12GraphicBuffer<DownSampleConstants>> downSampleCB;
			std::unique_ptr<JDx12GraphicBuffer<SsaoSampleConstants>> ssaoSampleCB;
			std::unique_ptr<JDx12GraphicBuffer<SsaoAoSliceConstants>> ssaoAoSliceCB;
		private:
			JUserPtr<JGraphicResourceInfo> randomVecInfo[(uint)J_SSAO_TYPE::COUNT];
			std::unordered_map<size_t, std::unique_ptr<SsaoIntermediate>> ssaoInterMap; 
		public:
			JDx12ImageProcessing();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicBaseDataSet& baseDataSet)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void ApplyBlur(JGraphicBlurComputeSet* computeSet, const JDrawHelper& helper) final;
			void ApplyMipmapGeneration(JGraphicDownSampleComputeSet* computeSet, const JDrawHelper& helper) final;
			void ApplySsao(JGraphicSsaoComputeSet* computeSet, const JDrawHelper& helper) final;
		private:
			//ssao process
			void DepthMapLinearize(const SSaoDrawDataSet& set, const JDrawHelper& helper);
			void DepthMapInetrleave(const SSaoDrawDataSet& set, const JDrawHelper& helper);
			void DrawSsaoMap(const SSaoDrawDataSet& set, const JDrawHelper& helper);
			void CombineSsaoMap(const SSaoDrawDataSet& set, const JDrawHelper& helper);
			void BlurSsao(const SSaoDrawDataSet& set, const JDrawHelper& helper);
		private: 
			void BindViewPortAndRect(JDx12GraphicDevice* device, ID3D12GraphicsCommandList* cmdList, ID3D12Resource* rtResource);
			void DrawFullScreenQuad(JDx12GraphicResourceManager* gm, ID3D12GraphicsCommandList* cmdList, ID3D12PipelineState* pso);
		public:
			void RecompileShader(const JGraphicShaderCompileSet& dataSet)final;
		public:
			void TryCreationSsaoIntermediate(JGraphicDevice* device, JGraphicResourceManager* gM, JGraphicResourceInfo* newSsao)final;
			void TryDestructionSsaoIntermediate(JGraphicDevice* device, JGraphicResourceManager* gM, JGraphicResourceInfo* ssao)final;
		private:
			void BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildBuffer(JDx12GraphicDevice* device, JDx12GraphicResourceManager* gm);
		private:
			void BuildBlurRootSignature(ID3D12Device* device);
			void BuildBlurPso(ID3D12Device* device, const JGraphicInfo& info);
			void BuildDownSampleRootSignature(ID3D12Device* device);
			void BuildDownSamplePso(ID3D12Device* device, const JGraphicInfo& info);
			void BuildSsaoRootSignature(ID3D12Device* device, const JGraphicOption& option);
			void BuildSsaoPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildSsaoBlurRootSignature(ID3D12Device* device);
			void BuildSsaoBlurPso(ID3D12Device* device, const JGraphicInfo& info);
			void BuildSsaoCombineRootSignature(ID3D12Device* device);
			void BuildSsaoCombinePso(ID3D12Device* device, const JGraphicInfo& info);
			void BuildSsaoDepthLinearizeRootSignature(ID3D12Device* device);
			void BuildSsaoDepthLinearizePso(ID3D12Device* device, const JGraphicInfo& info);
			void BuildSsaoDepthInterleaveRootSignature(ID3D12Device* device);
			void BuildSsaoDepthInterleavePso(ID3D12Device* device, const JGraphicInfo& info);
		private:
			void ClearRootSignature();
			void ClearPso();
			void ClearBuffer(); 
		};
	}
}