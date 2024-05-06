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
#include"../JLightCulling.h" 
#include"../../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../../Buffer/Dx/JDx12GraphicBuffer.h"
#include"../../../../Object/Component/Light/JLightType.h"

namespace JinEngine
{
	namespace Graphic
	{
		class JDx12GraphicDevice;
		class JGraphicResourceInfo;
		class JDx12GraphicResourceManager;
		class JDx12FrameResource;
		class JDx12CullingManager;  
		class JDx12CommandContext;

		//referenced by gpu pro7  ch 2.1 by Kevin Ortegren and Emil Persson
		class JDx12LightCulling : public JLightCulling
		{
		private:
			using JDx12GraphicShaderDataHolder = JDx12GraphicShaderDataHolder<1>;
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mGRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mCRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mDRootSignature;;
		private:
			std::unique_ptr<JDx12GraphicShaderDataHolder> drawLightShader[(uint)JLightType::GetLocalLightCount()];
			std::unique_ptr<JDx12ComputeShaderDataHolder> computeLightClusterShader;
			std::unique_ptr<JDx12ComputeShaderDataHolder> clearOffsetBufferShader;
			std::unique_ptr<JDx12GraphicShaderDataHolder> drawDebugShader;
		private:
			std::unique_ptr<JDx12GraphicBufferT<uint>> resultOutBuffer;
			std::unique_ptr<JDx12GraphicBufferT<uint>> resultOutClearBuffer;
			std::unique_ptr<JDx12GraphicBufferT<uint>> counterClearBuffer; 	 
		private:
			//for debugging
			std::unique_ptr<JDx12GraphicBufferT<uint>> offsetDebugBuffer;
			std::unique_ptr<JDx12GraphicBufferT<uint64>> listDebugBuffer;
		private:
			//Intermediate
			std::unique_ptr<JDx12GraphicBufferT<uint>> resultClearUploadBuffer;
			std::unique_ptr<JDx12GraphicBufferT<uint>> counterClearUploadBuffer;
		private:
			JUserPtr<JGraphicResourceInfo> lightRt[(uint)JLightType::GetLocalLightCount()];
		/*
		private:
			JUserPtr<JGraphicResourceInfo> lowSphereVertex;
			JUserPtr<JGraphicResourceInfo> lowSphereIndex;
			JUserPtr<JGraphicResourceInfo> lowConeVertex;
			JUserPtr<JGraphicResourceInfo> lowConeIndex;
		private: 
			uint lowSphereVertexCount;
			uint lowSphereIndexCount;
			uint lowConeVertexCount;
			uint lowConeIndexCount;
		*/
		public:
			~JDx12LightCulling();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) final;
			void Clear() final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;  
		private:
			bool HasDependency(const JGraphicInfo::TYPE type)const noexcept final;
			bool HasDependency(const JGraphicOption::TYPE type)const noexcept final;
		private:
			void NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)final;
			void NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)final;
		public:
			void NotifyNewClusterOption(JGraphicDevice* device) final;
			void NotifyLocalLightCapacityChanged(JGraphicDevice* device, JGraphicResourceManager* gM, const size_t capacity) final;
		public:
			void BindDrawResource(const JGraphicBindSet* bindSet) final; 
			void BindDebugResource(const JGraphicBindSet* bindSet) final;
		public:
			void ExecuteLightClusterTask(const JGraphicLightCullingTaskSet* taskSet, const JDrawHelper& helper)final;
			void ExecuteLightClusterDebug(const JGraphicLightCullingDebugDrawSet* drawSet, const JDrawHelper& helper)final;
		public: 
			void StreamOutDebugInfo(const std::wstring& path)final;
		public:
			void RecompileShader(const JGraphicShaderCompileSet& dataSet)final;
		private:
			void DrawLight(JDx12CommandContext* context, const JDrawHelper& helper); 
			void DrawLightClusterDebug(JDx12CommandContext* context, const JDrawHelper& helper);
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM);
			void BuildGraphicRootSignature(ID3D12Device* device);
			void BuildComputeRootSignature(ID3D12Device* device);
			void BuildDebugRootSignature(ID3D12Device* device);
			void BuildGraphicPso(ID3D12Device* device);
			void BuildComputePso(ID3D12Device* device);
			void BuildDebugPso(ID3D12Device* device);
			void BuildCounterClearBuffer(JDx12GraphicDevice* device);
			void BuildResultBuffer(JDx12GraphicDevice* device, const uint localLightCapacity);
			void BuildDebugBuffer(JDx12GraphicDevice* device);
			void BuildRtResource(JDx12GraphicDevice* device, JDx12GraphicResourceManager* dx12Gm);
		private:
			void ClearResource();
		private:
			void LoadLightShape(JDx12GraphicDevice* device, JDx12GraphicResourceManager* gM);
		};
	}
}