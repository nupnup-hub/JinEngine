#pragma once
#include"../JSceneDraw.h"
#include<d3d12.h>    
#include<vector>
#include<wrl/client.h>

namespace JinEngine
{
	namespace Graphic
	{
		struct JDrawCondition;
		class JDx12FrameResource;
		class JDx12CullingManager;
		class JDx12GraphicResourceManager;
		class JDx12GraphicShaderDataHolder;
		class JDx12GraphicDevice;
		class JDx12SceneDraw final : public JSceneDraw
		{
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			bool HasPreprocessing()const noexcept final;	//if true call BeginDraw before draw
			bool HasPostprocessing()const noexcept final;  //if true call EndDraw after draw
		public:
			void BindResource(const JGraphicBindSet* bindSet)final;
		public:
			//for multi thread
			//clear rtv dsv and set resource state
			void BeginDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)final;
			//for multi thread
			//set resource state
			void EndDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)final;
		public:
			//Contain Draw Debug UI
			//DrawSceneRenderTarget�� ��� camera�� hdocc ������ �ش� �Լ������� resource state�� predict�� �����Ѵ�.
			//���Ŀ� Single thread draw ���� Multi thread darw�� ���� BeginDraw. EndDraw�� ���� ��ó�� ������ �� �ʿ������� ���
			//Single Thread������ BeginDraw. EndDraw�� ��ó���� ó���ϵ��� �����ϵ��� �Ѵ�.
			void DrawSceneRenderTarget(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)final;
			void DrawSceneRenderTargetMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)final;
			void DrawSceneDebugUI(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)final;
		private:
			void DrawGameObject(ID3D12GraphicsCommandList* cmdList,
				JDx12FrameResource* dx12Frame,
				JDx12GraphicResourceManager* dx12Gm,
				JDx12CullingManager* dx12Cm,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper& helper,
				const JDrawCondition& condition);
		public:
			void NotifyChangeGraphicShaderMacro(JGraphicDevice* device, const JGraphicInfo& info)final;
			JOwnerPtr<JGraphicShaderDataHolderBase> CreateGraphicShader(JGraphicDevice* device, JGraphicResourceManager* graphicResourceM, const JGraphicShaderInitData& initData)final;
		private:
			void CompileShader(JDx12GraphicShaderDataHolder* holder, const JGraphicShaderInitData& initData);
			void StuffInputLayout(_Out_ std::vector<D3D12_INPUT_ELEMENT_DESC>& outInputLayout, const J_SHADER_VERTEX_LAYOUT vertexLayoutFlag);
			void StuffPso(JDx12GraphicShaderDataHolder* holder,
				JDx12GraphicDevice* dx12Device,
				JDx12GraphicResourceManager* dx12Gm,
				const J_GRAPHIC_SHADER_FUNCTION gFunctionFlag,
				const J_GRAPHIC_SHADER_EXTRA_FUNCTION extraType,
				const JShaderCondition& condition);
		private:
			void BuildRootSignature(ID3D12Device* device, const JGraphicInfo& info);
			void ReBuildRootSignature(ID3D12Device* device, const JGraphicInfo& info);
		};
	}
}