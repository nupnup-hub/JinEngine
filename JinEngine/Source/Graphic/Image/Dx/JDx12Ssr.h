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
#include"JDx12ImageConstants.h"
#include"../JSsr.h" 
#include"../../Accelerator/Dx/JDx12GpuAcceleratorManager.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceShareData.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../Buffer/Dx/JDx12GraphicBuffer.h" 
#include"../../../Core/Storage/JStorageInterface.h"
#include"../../../Core/Math/JMatrix.h"
#include"../../../Core/Interface/JActivatedInterface.h"
#include"../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
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
		class JDx12CommandContext;
		class JGraphicResourceInfo;
		struct JDx12PostProcessComputeSet;

		class JDx12Ssr final : public JSsr
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			struct SsrPassConstants
			{
			public:
				JMatrix4x4 camView = JMatrix4x4::Identity();
				JMatrix4x4 camProj = JMatrix4x4::Identity();
				JMatrix4x4 camInvView = JMatrix4x4::Identity();

				JVector2F rtSize = JVector2F::One();
				JVector2F invRtSize = JVector2F::One();

				//JVector2F halfRtSize = JVector2F::One();
				//JVector2F halfInvRtSize = JVector2F::One();

				JVector2F uvToViewA = JVector2F::One();
				JVector2F uvToViewB = JVector2F::One();

				JVector2F camNearFar = JVector2F::One();
				float camNearFarMul = 0;
				float startOffset = 0;

				JVector3F camPosW = JVector3F::PositiveOne();
				float stepScale = 0;

				float maxStepCount = 0;
				float rayDistance = 0;                  //0 ~ 10000   
				float thickness = 0;                    //0 ~ 10000
				float objectViewZBias = 0;              //0 ~ 10000

				float fadeDistance = 0;                 //0 ~ 1.0f
				float fadeOneRate = 0;                  //to [0.0f ~ 1.0f]
				int pad00 = 0;
				int pad01 = 0;
			};
		private:
			struct UserPrivateData : public GraphicVolatileStorageInterface
			{
			public:
				JDx12GraphicBufferT<SsrPassConstants> frameBuffer;
			public:
				UserPrivateData(JGraphicDevice* device);
				~UserPrivateData();
			public:
				void Begin(const JSsrDesc& desc, const JDrawHelper& helper);
				void End(const JDrawHelper& helper);
			};
			struct SsrComputeSet
			{
			public:
				UserPrivateData* userPrivate;
			public:
				JDx12CommandContext* context;
				JDx12GraphicDevice* device;
			public:
				ImageProcessingShareData* imageShare = nullptr;
				DrawSceneShareData* drawSceneShare = nullptr;
			public:
				JSsrDesc ssrDesc;
			public:
				int skyMapSrvHeapIndex = invalidIndex;
				bool isValid = false;
			public:
				JDx12GraphicResourceComputeSet rtSet;
				JDx12GraphicResourceComputeSet viewZSet;
				JDx12GraphicResourceComputeSet albedoSet;
				JDx12GraphicResourceComputeSet normalSet;
				JDx12GraphicResourceComputeSet lightPropSet;
				JDx12GraphicResourceComputeSet ssrSet;
				JDx12GraphicResourceComputeSet ssrMip[Constants::ssrMipCount];
			public:
				JDx12GraphicResourceComputeSet srcSet;
				JDx12GraphicResourceComputeSet intermediateSet;
				JDx12GraphicResourceComputeSet destSet;
			public:
				JDx12GraphicResourceComputeSet depthDerivativeMap; 
			public:
				JDx12AcceleratorResourceComputeSet accelSet;
			public:
				JVector2<uint> resolution;
				//JVector2<uint> halfResolution;
			public:
				SsrComputeSet(JDx12PostProcessComputeSet* computeSet, const JDrawHelper& helper);
			public:
				void SetUserPrivate(UserPrivateData* data, const JDrawHelper& helper);
			public:
				bool IsValid()const noexcept;
			};
		private:
			const size_t guid;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> computeRootSignature[(uint)J_SSR_TYPE::COUNT];
			Microsoft::WRL::ComPtr<ID3D12RootSignature> downSampleRootsignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> blurRootsignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> applyRootsignature;
		private:
			std::unique_ptr<JDx12ComputeShaderDataHolder> computeShader[(uint)J_SSR_TYPE::COUNT];
			std::unique_ptr<JDx12ComputeShaderDataHolder> downSample4Shader;
			std::unique_ptr<JDx12ComputeShaderDataHolder> downSample2Shader;
			std::unique_ptr<JDx12ComputeShaderDataHolder> blurShader;
			std::unique_ptr<JDx12ComputeShaderDataHolder> applyShader; 
		private:
			JDx12GraphicDevice* cachedDevice;
			JDx12GraphicResourceManager* cachedGm;
		private:
			std::unordered_map<size_t, std::unique_ptr<UserPrivateData>> userPrivate;
			uint computeCount = 0;
		public:
			JDx12Ssr();
			~JDx12Ssr();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		private:
			bool HasDependency(const JGraphicInfo::TYPE type)const noexcept final;
			bool HasDependency(const JGraphicOption::TYPE type)const noexcept final;
			bool HasDrawSequencePostProcessing()const noexcept final;
		private:
			void DrawSequencePostProcessing()final;
		private:
			void NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)final;
			void NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)final;
		public:
			void ApplySsr(JPostProcessComputeSet* computeSet, const JDrawHelper& helper) final;
		private:
			void ComputeWithRaymarching(SsrComputeSet& set, const JDrawHelper& helper);
			void ComputeWithRaycasting(SsrComputeSet& set, const JDrawHelper& helper);
			void DownSample(SsrComputeSet& set, const JDrawHelper& helper);
			void Blur(SsrComputeSet& set, const JDrawHelper& helper);
			void Apply(SsrComputeSet& set, const JDrawHelper& helper);
		private:
			void Begin(SsrComputeSet& set, const JDrawHelper& helper);
			void End(const SsrComputeSet& set, const JDrawHelper& helper);
		public:
			void RecompileShader(const JGraphicShaderCompileSet& dataSet)final;
		private:
			void BuildResouce(JGraphicDevice* device, JGraphicResourceManager* gM);
			void BuildRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
		private:
			void ClearResource();
			void ClearRootSignature();
			void ClearPso();
		};
	}
}