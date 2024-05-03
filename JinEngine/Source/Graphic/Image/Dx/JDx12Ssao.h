#pragma once
#include"JDx12ImageConstants.h"
#include"../JSsao.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceShareData.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
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
		class JDx12GraphicResourceManager;
		class JDx12GraphicDevice; 
		class JDx12FrameResource;
		class JDx12GraphicResourceShareData;
		class JDx12CommandContext;
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
		class JDx12Ssao : public JSsao
		{
		private:
			using JDx12GraphicShaderDataHolder = JDx12GraphicShaderDataHolder<1>;
		private: 
			struct SsaoSampleConstants
			{
				JVector4F sample[Constants::ssaoMaxSampleCount];
			};
			struct SsaoAoSliceConstants
			{
				JVector4F jitter;
				JVector2F posOffset;
				float sliceIndex;
				uint uSliceIndex;
			};
		private:
			struct SSaoDrawDataSet
			{ 
			public:
				JDx12CommandContext* context;
			public:
				JSsaoDesc ssaoDesc;
				bool canBlur;
				bool canUseHbaoInterleave = false;
				bool isValid = false;
			public: 
				SsaoShareData* inter = nullptr;
			public:
				JDx12GraphicResourceComputeSet rtSet;
				JDx12GraphicResourceComputeSet dsSet;
				JDx12GraphicResourceComputeSet normalSet;
				JDx12GraphicResourceComputeSet aoSet;
				JDx12GraphicResourceComputeSet aoInter00Set;
				JDx12GraphicResourceComputeSet aoInter01Set;
				JDx12GraphicResourceComputeSet aoInterleaveSet;
				JDx12GraphicResourceComputeSet aoDepthSet;
				JDx12GraphicResourceComputeSet aoDepthInterleaveSet;
				JDx12GraphicResourceComputeSet randomSet;
			public:
				SSaoDrawDataSet(JGraphicSsaoComputeSet* computeSet,
					const JDrawHelper& helper, 
					const JUserPtr<JGraphicResourceInfo>(&randomInfoVec)[(uint)J_SSAO_TYPE::COUNT]);
			};
		private:
			const size_t guid;
		private: 
			Microsoft::WRL::ComPtr<ID3D12RootSignature> ssaoRootSignature; 
			Microsoft::WRL::ComPtr<ID3D12RootSignature> hbaoRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> ssaoBlurRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> ssaoCombineRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> ssaoDepthLinearizeRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> ssaoDepthInterleaveRootSignature;
		private: 
			std::unique_ptr<JDx12GraphicShaderDataHolder> ssao[(uint)J_SSAO_TYPE::COUNT * (uint)J_SSAO_SAMPLE_TYPE::COUNT * (uint)J_SSAO_FUNCTION::COUNT];
			std::unique_ptr<JDx12GraphicShaderDataHolder> ssaoBlur[(uint)J_SSAO_BLUR_SHADER::COUNT][(uint)J_KERNEL_SIZE::COUNT];
			std::unique_ptr<JDx12GraphicShaderDataHolder> ssaoCombine[(uint)J_SSAO_FUNCTION::COUNT];
			std::unique_ptr<JDx12GraphicShaderDataHolder> ssaoDepthLinearize;
			std::unique_ptr<JDx12GraphicShaderDataHolder> ssaoDepthInterleave;
		private:
			//inner buffer 
			std::unique_ptr<JDx12GraphicBufferT<SsaoSampleConstants>> ssaoSampleCB;
			std::unique_ptr<JDx12GraphicBufferT<SsaoAoSliceConstants>> ssaoAoSliceCB;
		private:
			JUserPtr<JGraphicResourceInfo> randomVecInfo[(uint)J_SSAO_TYPE::COUNT]; 
		public:
			JDx12Ssao();
			~JDx12Ssao();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		private:
			bool HasDependency(const JGraphicInfo::TYPE type)const noexcept final;
			bool HasDependency(const JGraphicOption::TYPE type)const noexcept final;
		private:
			void NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)final;
			void NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)final;
		public: 
			void ApplySsao(JGraphicSsaoComputeSet* computeSet, const JDrawHelper& helper) final;
		private:
			//ssao process
			void DepthMapLinearize(const SSaoDrawDataSet& set, const JDrawHelper& helper);
			void DepthMapInetrleave(SSaoDrawDataSet& set, const JDrawHelper& helper);
			void DrawSsaoMap(SSaoDrawDataSet& set, const JDrawHelper& helper);
			void CombineSsaoMap(const SSaoDrawDataSet& set, const JDrawHelper& helper);
			void BlurSsao(const SSaoDrawDataSet& set, const JDrawHelper& helper);
		public:
			void RecompileShader(const JGraphicShaderCompileSet& dataSet)final;
		private:
			void BuildResouce(JGraphicDevice* device, JGraphicResourceManager* gM);
			void BuildRootSingnature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildBuffer(JDx12GraphicDevice* device, JDx12GraphicResourceManager* gm);
		private: 
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
			void ClearResource();
			void ClearRootSignature();
			void ClearPso();
			void ClearBuffer();
		};
	}
}