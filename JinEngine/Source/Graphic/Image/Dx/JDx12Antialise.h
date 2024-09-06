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
#include"../JAntialise.h" 
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceShareData.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include"../../Buffer/Dx/JDx12GraphicBuffer.h"
#include"../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include"../../../Core/Math/JMatrix.h"
#include"../../../Core/Storage/JStorageInterface.h"  
#include"../../../Core/Interface/JActivatedInterface.h"  
#include<wrl/client.h> 
#include<dxgiformat.h>

namespace JinEngine
{ 
	class JCamera;
	namespace Graphic
	{ 
		class JDx12GraphicResourceManager;
		class JDx12GraphicDevice;
		class JDx12CommandContext;

		class JDx12Antialise final: public JAntialise
		{
		private:
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			static constexpr uint aaVariation = 2;
		private:
			struct TAAPassConstants
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
				uint sampleNumber = 0; 
			};
			struct TAAUserPrivateData : public Core::JVolatileStorageInterface
			{
			public:
				static constexpr uint historyCount = 2;
			public:
				JUserPtr<JGraphicResourceInfo> colorHistory[historyCount]; 
			public:
				JUserPtr<JGraphicResourceInfo> viewZ;			//sample 연산중 중복되는 계산을 피하기 위해
				JUserPtr<JGraphicResourceInfo> preViewZ;		//sample 연산중 중복되는 계산을 피하기 위해
			public:
				JGraphicDevice* device = nullptr;
				JGraphicResourceManager* gm = nullptr;
			public:
				JDx12GraphicBufferT<TAAPassConstants> frameBuffer;
			public:
				uint historyIndex = 0;
				uint preHistoryIndex = 1;
			public:
				bool waitResourceCreation = true;
			public:
				TAAUserPrivateData(JGraphicDevice* device);
				~TAAUserPrivateData();
			public:
				void Begin(const JDrawHelper& helper);
				void End(const JDrawHelper& helper);
			};
			struct TAADataSet 
			{
			public:
				JDx12CommandContext* context;
				JDx12GraphicDevice* device;
				JGraphicResourceManager* gm;
				TAAUserPrivateData* userPrivate; 
			public:
				ImageProcessingShareData* imageShare = nullptr;
			public:
				JUserPtr<JCamera> cam;
			public:
				JDx12GraphicResourceComputeSet rtSet;
				JDx12GraphicResourceComputeSet dsSet;
				JDx12GraphicResourceComputeSet viewZSet;
			public:
				JDx12GraphicResourceComputeSet lightPropSet;
				JDx12GraphicResourceComputeSet normalSet;
				JDx12GraphicResourceComputeSet velocitySet;
			public:
				JDx12GraphicResourceComputeSet preLightPropSet;
				JDx12GraphicResourceComputeSet preNormalSet;
				JDx12GraphicResourceComputeSet preDepthSet;
				JDx12GraphicResourceComputeSet preViewZSet;
			public:
				JDx12GraphicResourceComputeSet colorSet; 
				JDx12GraphicResourceComputeSet colorHistorySet;   
				JDx12GraphicResourceComputeSet preColorHistorySet; 
			public:
				JDx12GraphicResourceComputeSet destSet;
			public:
				JDx12GraphicResourceComputeSet* taSrc = nullptr;
				JDx12GraphicResourceComputeSet* taPreHistory = nullptr;
				JDx12GraphicResourceComputeSet* taCurHistory = nullptr;
			public: 
				JDx12GraphicResourceComputeSet* sharpHistory = nullptr;
				JDx12GraphicResourceComputeSet* sharpDest = nullptr;
			public:
				JVector2<uint> resolution;
			public:
				int currFrameIndex;
			public:
				bool requestCreateDependencyData = false;
			public:
				TAADataSet(JPostProcessComputeSet* computeSet, const JDrawHelper& helper);
			public:
				void SetUserPrivate(TAAUserPrivateData* data, const JDrawHelper& helper);
			};
		private:
			class AABase : public Core::JActivatedInterface
			{
			public:
				void BuildResource(JGraphicDevice* device, const JGraphicInfo& info, const JGraphicOption& option);
			private:
				virtual void BuildSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option) = 0;
				virtual void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option) = 0;
			public:
				void ClearResource();
			private:
				virtual void ClearSignature() = 0;
				virtual void ClearPso() = 0;
			};
			class FxaaResource final : public AABase
			{
			public:
				Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
				Microsoft::WRL::ComPtr<ID3D12CommandSignature> commandSignature;
			public:
				std::unique_ptr<JDx12ComputeShaderDataHolder> pass1InputLinearColor;
				std::unique_ptr<JDx12ComputeShaderDataHolder> pass1InputLuma;
				std::unique_ptr<JDx12ComputeShaderDataHolder> pass2Vertical;
				std::unique_ptr<JDx12ComputeShaderDataHolder> pass2Horizontal;
				std::unique_ptr<JDx12ComputeShaderDataHolder> resolveWork;
			public: 
				void BuildSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)final; 
				void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)final;
			private:
				void BuildRootSignature(ID3D12Device* device);
				void BuildCommandSignature(ID3D12Device* device);
			public: 
				void ClearSignature()final; 
				void ClearPso()final;
			public: 
				void Execute(JPostProcessComputeSet* computeSet, const JDrawHelper& helper);
			};
			class TaaResource final : public AABase
			{
			private: 
				Microsoft::WRL::ComPtr<ID3D12RootSignature> prepareRootSignature;
				Microsoft::WRL::ComPtr<ID3D12RootSignature> taRootSignature;
				Microsoft::WRL::ComPtr<ID3D12RootSignature> sharpeningRootSignature;
			private:
				std::unique_ptr<JDx12ComputeShaderDataHolder> prepare;
				std::unique_ptr<JDx12ComputeShaderDataHolder> ta;
				std::unique_ptr<JDx12ComputeShaderDataHolder> sharpening;
			private:
				Microsoft::WRL::ComPtr<ID3D12RootSignature> clearRootSignature;
				std::unique_ptr<JDx12ComputeShaderDataHolder> clearShader;
			public: 
				void BuildSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)final;
				void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option)final;
			public: 
				void ClearSignature()final;
				void ClearPso()final; 
			public:
				void Prepare(TAADataSet& set, const JDrawHelper& helper);
				void TemporalAccumulation(TAADataSet& set, const JDrawHelper& helper);
				void Sharpening(TAADataSet& set, const JDrawHelper& helper);
				void ClearTAAResource(TAADataSet& set, const JDrawHelper& helper);
			public:
				void SettingFirstLoop(TAADataSet& set, const JDrawHelper& helper);
			}; 
		private:
			FxaaResource fxaa;
			TaaResource taa;
		private:
			AABase* aaBase[aaVariation];
		private:
			PushGraphicEventPtr pushGraphicEvPtr;
		private:
			std::unordered_map<size_t, std::unique_ptr<TAAUserPrivateData>> taaUserPrivate;
			uint computeCount = 0;
		public: 
			JDx12Antialise(PushGraphicEventPtr pushGraphicEvPtr);
			~JDx12Antialise();
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
			void ApplyFXAA(JPostProcessComputeSet* computeSet, const JDrawHelper& helper)final;
			void ApplyTAA(JPostProcessComputeSet* computeSet, const JDrawHelper& helper) final;
		private:
			void BeginTAA(TAADataSet& set, const JDrawHelper& helper);
			void EndTAA(TAADataSet& set, const JDrawHelper& helper);
		public:
			void RecompileShader(const JGraphicShaderCompileSet& dataSet) final;
		private:
			void CreateDependencyData(JGraphicDevice* device, JGraphicResourceManager* gm, TAAUserPrivateData* userPrivate, JVector2<uint> rtSize);
		private:
			void BuildResource(JGraphicDevice* device);
		private:
			void ClearResource(); 
		};
	}
}