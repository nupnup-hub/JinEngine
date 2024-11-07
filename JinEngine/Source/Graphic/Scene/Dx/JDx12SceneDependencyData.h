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
#include"../JSceneDependencyData.h"
#include"../../DataSet/Dx/JDx12GraphicTaskDataSet.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceShareData.h"
#include"../../GraphicResource/JGraphicResourceInterface.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"   
#include"../../../Core/Math/JMatrix.h"
#include<wrl/client.h>

namespace JinEngine
{
	namespace Graphic
	{ 
		class JDx12CommandContext;
		class JDx12FrameResource;
		class JDx12CullingManager;
		class JDx12GraphicResourceManager;
		class JDx12GraphicDevice; 

		class JDx12SceneDependencyData final : public JSceneDependencyData
		{
		private:
			using JDx12GraphicShaderDataHolder = JDx12GraphicShaderDataHolder<(uint)J_GRAPHIC_SHADER_EXTRA_FUNCTION::COUNT>;
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			struct DepthReleativeDataSet
			{
			public:
				JDx12GraphicDevice* device;
				JGraphicResourceManager* gm;
			public:
				JDx12GraphicResourceShareData* shareData = nullptr;
				DrawSceneShareData* drawSceneShareData = nullptr;
			public:
				JDx12GraphicResourceComputeSet dsSet;
				JDx12GraphicResourceComputeSet preDsSet;
				JDx12GraphicResourceComputeSet viewZSet;
				JDx12GraphicResourceComputeSet preViewZSet; 
				JDx12GraphicResourceComputeSet depthDerivativeSet;   
			public:
				JVector2<uint> resolution;
			public:
				JWeakPtr<JCamera> cam;
				int camFrameIndex = 0;
			public:
				DepthReleativeDataSet(const JDx12GraphicSceneDependencyDataComputeSet* set, const JDrawHelper& helper);
			public:
				bool IsValid()const noexcept;
			};
			struct VelocityDataSet
			{
			public:
				JDx12GraphicDevice* device;
				JGraphicResourceManager* gm;
			public:
				JDx12GraphicResourceShareData* shareData = nullptr;
				DrawSceneShareData* drawSceneShareData = nullptr;
			public:
				JDx12GraphicResourceComputeSet rtSet;
				JDx12GraphicResourceComputeSet viewZSet;
				JDx12GraphicResourceComputeSet velocitySet;
			public:
				DirectX::XMMATRIX camPreViewProj;
			public: 
				int camFrameIndex = invalidIndex;
			public:
				VelocityDataSet(const JDx12GraphicSceneDependencyDataComputeSet* set, const JDrawHelper& helper);
			public:
				bool IsValid()const noexcept;
			};
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> depthRootsignature;
			std::unique_ptr<JDx12ComputeShaderDataHolder> depthShader;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> velocityRootsignature;
			std::unique_ptr<JDx12ComputeShaderDataHolder> velocityShader;
		private:
			PushGraphicEventPtr pushGraphicEvPtr;
		public:
			JDx12SceneDependencyData(PushGraphicEventPtr pushGraphicEvPtr);
			~JDx12SceneDependencyData();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;  
		public:
			void ComputeDepthRelative(const JGraphicSceneDependencyDataComputeSet* computeSet, const JDrawHelper& helper) final;
			void ComputeVelocity(const JGraphicSceneDependencyDataComputeSet* computeSet, const JDrawHelper& helper) final;
		private:
			void RequestShareDataCreation(JGraphicDevice* device, JGraphicResourceManager* gm, JDx12GraphicResourceShareData* shareData, JWeakPtr<JCamera> cam);
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM);
			void BuildRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildPso(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
		private:
			void ClearResource();
		};
	}
}