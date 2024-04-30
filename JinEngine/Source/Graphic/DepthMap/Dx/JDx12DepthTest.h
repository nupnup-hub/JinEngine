#pragma once
#include"../JDepthTest.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../../Core/Utility/JTypeTraitUtility.h" 
#include"../../../Core/Geometry/Mesh/JMeshType.h"  
#include"../../../../ThirdParty/DirectX/Tk/Src/d3dx12.h"
#include<wrl/client.h>

namespace JinEngine
{
	namespace Graphic
	{ 
		class JDx12FrameResource;
		class JDx12CommandContext;
		class JDx12DepthTest final : public JDepthTest
		{
		private:
			using JDx12GraphicShaderDataHolder = JDx12GraphicShaderDataHolder<1>;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
			Core::EnumCountArray<std::unique_ptr<JDx12GraphicShaderDataHolder>, TEST_TYPE> gShaderData;
			//std::unique_ptr<JGraphicShaderData> gShaderData[(uint)TEST_TYPE::COUNT];
		public:
			~JDx12DepthTest();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			void DrawSceneBoundingBox(const JGraphicDepthMapDrawSet* drawSet,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper& helper,
				const JDrawCondition& condition)final;
			void DrawHdOcclusionQueryObject(const JGraphicDepthMapDrawSet* drawSet,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper& helper,
				const JDrawCondition& condition) final;
		public:
			bool BindGraphicResource(const JGraphicBindSet* bindSet, const JDrawHelper& helper) final;
		private:
			bool BindGraphicResource(JDx12CommandContext* context, const JDrawHelper& helper);
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info);
			void BuildRootSignature(ID3D12Device* device);
			void BuildPso(ID3D12Device* device, const DXGI_FORMAT depthStencilFormat);
		private:
			void ClearResource();
		};
	}
}
 