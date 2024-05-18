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
#include"../JRaytracingDenoiser.h" 
#include"../../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../../GraphicResource/Dx/JDx12GraphicResourceShareData.h"
#include"../../../Buffer/Dx/JDx12GraphicBuffer.h"  
#include"../../../Shader/Dx/JDx12ShaderDataHolder.h"   
#include"../../../../Core/Storage/JStorageInterface.h"
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h" 
#include"../../../../Core/Math/JMatrix.h"
#include"../../../../Core/Interface/JActivatedInterface.h"
#include<wrl/client.h>  
#include<random>

namespace JinEngine
{
	class JCamera;
	namespace Graphic
	{
		using RestirTemporalAccumulationData = JDx12GraphicResourceShareData::RestirTemporalAccumulationData;
		class JDx12CommandContext;
		class JDx12RaytracingDenoiser : public JRaytracingDenoiser
		{
		private:
			static constexpr uint denoiserCount = 2;
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			struct GIDenoiserPassConstants
			{
			public:
				JMatrix4x4 camInvView = JMatrix4x4::Identity();
				JMatrix4x4 camPreInvView = JMatrix4x4::Identity();
				JMatrix4x4 camPreViewProj = JMatrix4x4::Identity();

				JVector2F rtSize = JVector2F::One();
				JVector2F invRtSize = JVector2F::One();

				JVector2F uvToViewA = JVector2F::One();
				JVector2F uvToViewB = JVector2F::One();

				JVector2F preUvToViewA = JVector2F::One();
				JVector2F preUvToViewB = JVector2F::One();

				JVector2F camNearFar = JVector2F::One();
				float camNearMulFar = 0;
				float denoiseRange = 0;

				float baseRadius = 0;
				float radiusRange = 0;
				uint sampleNumber = 0;
				uint pad00 = 0;
			};
		private:
			struct UserPrivateData : public Core::JVolatileStorageInterface
			{
			public:
				static constexpr uint historyCount = 2;
			public:
				JUserPtr<JGraphicResourceInfo> colorHistory[historyCount];
				JUserPtr<JGraphicResourceInfo> fastColorHistory[historyCount];
				JUserPtr<JGraphicResourceInfo> historyLength[historyCount];  
			public:
				JDx12GraphicBufferT<GIDenoiserPassConstants> frameBuffer;
			private:
				JMatrix4x4 camPreInvView = JMatrix4x4::Identity();
				JVector2F preUvToViewA = JVector2F::One();
				JVector2F preUvToViewB = JVector2F::One();
			private:
				std::random_device rd;	//use hardware
				std::mt19937 gen;
				std::uniform_real_distribution<float> disUNorm;
			public:
				uint historyIndex = 0;
				uint preHistoryIndex = 1;
			public:
				bool waitResourceCreation = true;
			public:
				UserPrivateData(JGraphicDevice* device);
				~UserPrivateData();
			public:
				void Begin(const JDrawHelper& helper);
				void End(const JDrawHelper& helper);
			};
			struct DenoiseDataSet
			{
			public:
				JDx12CommandContext* context;
				JDx12GraphicDevice* device;
				JGraphicResourceManager* gm;
				UserPrivateData* userPrivate;
			public:
				RestirTemporalAccumulationData* sharedata;
			public:
				JUserPtr<JCamera> cam;
			public:
				JDx12GraphicResourceComputeSet rtSet;
				JDx12GraphicResourceComputeSet dsSet;
				JDx12GraphicResourceComputeSet viewZSet;
			public:
				JDx12GraphicResourceComputeSet lightPropSet;
				JDx12GraphicResourceComputeSet normalSet;
				//JDx12GraphicResourceComputeSet velocitySet;
			public:
				JDx12GraphicResourceComputeSet preLightPropSet;
				JDx12GraphicResourceComputeSet preNormalSet;
				JDx12GraphicResourceComputeSet preDepthSet;
				JDx12GraphicResourceComputeSet preViewZSet;
			public:
				JDx12GraphicResourceComputeSet colorSet;
			public:
				JDx12GraphicResourceComputeSet colorHistorySet; 
				JDx12GraphicResourceComputeSet fastColorHistorySet;
				JDx12GraphicResourceComputeSet historyLengthSet;
			public:
				JDx12GraphicResourceComputeSet preColorHistorySet; 
				JDx12GraphicResourceComputeSet preFastColorHistorySet;
				JDx12GraphicResourceComputeSet preHistoryLengthSet;
			public:
				JDx12GraphicResourceComputeSet colorHistoryIntermediateSet00; 
				JDx12GraphicResourceComputeSet colorHistoryIntermediateSet01; 
				JDx12GraphicResourceComputeSet denoiseMipmapSet[Constants::restirDenoiseMipmapCount];
			public:
				JDx12GraphicResourceComputeSet depthDerivative;
			public:
				JDx12GraphicResourceComputeSet* srcColor = nullptr;
				JDx12GraphicResourceComputeSet* destColor = nullptr;
				JDx12GraphicResourceComputeSet* colorHistory = nullptr;
				JDx12GraphicResourceComputeSet* preColorHistory = nullptr;
				JDx12GraphicResourceComputeSet* fastColorHistory = nullptr;
				JDx12GraphicResourceComputeSet* preFastColorHistory = nullptr; 
				JDx12GraphicResourceComputeSet* intermediate00 = nullptr;
				JDx12GraphicResourceComputeSet* intermediate01 = nullptr;
			public:
				JVector2<uint> resolution;
			public:
				int currFrameIndex;
			public:
				bool requestCreateDependencyData = false;
			public:
				DenoiseDataSet(const JGraphicRtDenoiseComputeSet* computeSet, const JDrawHelper& helper);
			public:
				void SetUserPrivate(UserPrivateData* data, const JDrawHelper& helper);
			public:
				bool IsValid()const noexcept;
			};
		private:
			class DenoiserBase : public Core::JActivatedInterface
			{ 
			public:
				void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM);
			private:
				virtual void BuildRootSignature(JDx12GraphicDevice* device) = 0;
				virtual void BuildPso(JDx12GraphicDevice* device) = 0;
			public:
				void ClearResource();
			private:
				virtual void ClearRootSignature() = 0;
				virtual void ClearPso() = 0;
			};
			class RestirDenoiser : public DenoiserBase
			{
				//for restir gi
			private:
				Microsoft::WRL::ComPtr<ID3D12RootSignature> prepareRootSignature;
				Microsoft::WRL::ComPtr<ID3D12RootSignature> taRootSignature;
				Microsoft::WRL::ComPtr<ID3D12RootSignature> historyFixRootSignature;
				Microsoft::WRL::ComPtr<ID3D12RootSignature> historyClampingRootSignature;
				Microsoft::WRL::ComPtr<ID3D12RootSignature> antiFireflyRootSignature;
				Microsoft::WRL::ComPtr<ID3D12RootSignature> atorusRootSignature;
				Microsoft::WRL::ComPtr<ID3D12RootSignature> historyStabilizationRootSignature;
			private:
				std::unique_ptr<JDx12ComputeShaderDataHolder> prepareShader;
				std::unique_ptr<JDx12ComputeShaderDataHolder> taShader;
				std::unique_ptr<JDx12ComputeShaderDataHolder> historyFixShader;
				std::unique_ptr<JDx12ComputeShaderDataHolder> historyClampingShader;
				std::unique_ptr<JDx12ComputeShaderDataHolder> antiFireflyShader;
				std::unique_ptr<JDx12ComputeShaderDataHolder> atorusShader;
				std::unique_ptr<JDx12ComputeShaderDataHolder> historyStabilizationShader;
			private:
				Microsoft::WRL::ComPtr<ID3D12RootSignature> clearRootSignature;
				std::unique_ptr<JDx12ComputeShaderDataHolder> clearShader;
			public:
				~RestirDenoiser();  
			private:
				void BuildRootSignature(JDx12GraphicDevice* device) final;
				void BuildPso(JDx12GraphicDevice* device) final;
			private:
				void ClearRootSignature() final;
				void ClearPso() final;
			public:
				void Prepare(const DenoiseDataSet& set, const JDrawHelper& helper);
				void TemporalAccumulation(const DenoiseDataSet& set, const JDrawHelper& helper);
				void HistoryFix(const DenoiseDataSet& set, const JDrawHelper& helper);
				void HistoryClamping(const DenoiseDataSet& set, const JDrawHelper& helper);
				void AnitiFirefly(const DenoiseDataSet& set, const JDrawHelper& helper);
				//void GiBlurHotHistory(const DenoiseDataSet& set, const JDrawHelper& helper);
				//void GiDownSampling(DenoiseDataSet& set, const JDrawHelper& helper); 
				//void GiReconstructHistory(DenoiseDataSet& set, const JDrawHelper& helper);
				void Atorus(DenoiseDataSet& set, const JDrawHelper& helper, const uint stepCount);
				void HistoryStabilization(const DenoiseDataSet& set, const JDrawHelper& helper);
				void ClearDenoiseResource(const DenoiseDataSet& set, const JDrawHelper& helper);
			public:
				void SettingFirstLoop(DenoiseDataSet& set, const JDrawHelper& helper);
				void SettingSecondLoop(DenoiseDataSet& set, const JDrawHelper& helper);
				void SettingThirdLoop(DenoiseDataSet& set, const JDrawHelper& helper);
			};
			class ReCurrentDenoiser : public DenoiserBase
			{
				//common deoniser
			public:
				~ReCurrentDenoiser(); 
			private:
				void BuildRootSignature(JDx12GraphicDevice* device)final;
				void BuildPso(JDx12GraphicDevice* device)final;
			private:
				void ClearRootSignature()final;
				void ClearPso();
			};
		private:
			RestirDenoiser restirDenoiser;
			ReCurrentDenoiser reCurrentDenoiser;
		private:
			DenoiserBase* denoiser[denoiserCount];
		private:
			std::unordered_map<size_t, std::unique_ptr<UserPrivateData>> userPrivate;
			uint computeCount = 0;
		private:
			PushGraphicEventPtr pushGraphicEvPtr;
		public:
			JDx12RaytracingDenoiser(PushGraphicEventPtr pushGraphicEvPtr);
			~JDx12RaytracingDenoiser();
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
			void ApplyGIDenoise(const JGraphicRtDenoiseComputeSet* computeSet, const JDrawHelper& helper);
		private:
			void Begin(DenoiseDataSet& set, const JDrawHelper& helper);
			void End(const DenoiseDataSet& set, const JDrawHelper& helper);
		private:
			void CreateDependencyData(JGraphicDevice* device, JGraphicResourceManager* gm, UserPrivateData* userPrivate, JVector2<uint> rtSize);
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM); 
		private:
			//void BuildBuffer(JDx12GraphicDevice* device);
		private:
			void ClearResource(); 
			//void ClearUserPrivateData();
		};
	}
}