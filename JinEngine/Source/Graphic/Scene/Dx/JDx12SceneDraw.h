#pragma once
#include"../JSceneDraw.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
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
		public:
			enum class INNER_DEFERRED_SHADER_TYPE
			{
				STANDARD,
				EXCEPT_LIGHT_CULLING,
				COUNT
			};
		private:
			struct PsoBuildData
			{
			public:
				JDx12GraphicShaderDataHolder* holder;
				JDx12GraphicDevice* dx12Device; 
				J_GRAPHIC_SHADER_TYPE shaderType;
				J_GRAPHIC_SHADER_FUNCTION gFunctionFlag;
				J_GRAPHIC_SHADER_EXTRA_FUNCTION extraType;
				J_GRAPHIC_RENDERING_PROCESS processType;
				JShaderCondition condition; 
			};
			struct GraphicDataSet
			{
			public:
				//device data
				JDx12GraphicDevice* dx12Device = nullptr;
				JDx12FrameResource* dx12Frame = nullptr;
				JDx12GraphicResourceManager* dx12Gm = nullptr;
				JDx12CullingManager* dx12Cm = nullptr;
				ID3D12GraphicsCommandList* cmdList = nullptr;
			public:
				GraphicDataSet(const JDx12GraphicBindSet* dx12BindSet);
				GraphicDataSet(const JDx12GraphicSceneDrawSet* dx12DrawSet);
			};
			struct ResourceDataSet
			{  
			public:
				//Drawing data
				uint scenePassCBIndex = 0;
				uint camCBIndex = 0;
				int camFrameIndex = invalidIndex;
				int sceneFrameIndex = invalidIndex; 
			public:
				ID3D12Resource* dsResource = nullptr;
				ID3D12Resource* rtResource = nullptr;
				ID3D12Resource* albedoResource = nullptr;
				ID3D12Resource* normalResource = nullptr;
				ID3D12Resource* tangentResource = nullptr;
				ID3D12Resource* lightPropResource = nullptr;
			public:
				CD3DX12_CPU_DESCRIPTOR_HANDLE dsv;
				CD3DX12_CPU_DESCRIPTOR_HANDLE rtv;
				CD3DX12_GPU_DESCRIPTOR_HANDLE albedoSrv;
				CD3DX12_CPU_DESCRIPTOR_HANDLE albedoRtv;
				CD3DX12_CPU_DESCRIPTOR_HANDLE normalRtv;
				CD3DX12_CPU_DESCRIPTOR_HANDLE tangentRtv;
				CD3DX12_CPU_DESCRIPTOR_HANDLE lightPropRtv;
			public:
				//Debug
				int debugVecIndex = invalidIndex;
				int debugHeapIndex = invalidIndex;
				ID3D12Resource* debugResource = nullptr;
				CD3DX12_CPU_DESCRIPTOR_HANDLE debugDsv;
			public:
				//Clsuter
				int dsSrvHeapIndex = invalidIndex;
				int offsetSrvHeapIndex = invalidIndex;
				int linkedSrvHeapIndex = invalidIndex;
				CD3DX12_GPU_DESCRIPTOR_HANDLE depthMapSrv;
				CD3DX12_GPU_DESCRIPTOR_HANDLE clusterLinkOffsetSrv;
				CD3DX12_GPU_DESCRIPTOR_HANDLE clusterLinkListSrv;
			public:
				bool canUseLightCulling = false;
				bool canUseLightCluster = false;
			public:
				ResourceDataSet(const GraphicDataSet& set, const JDrawHelper& helper);
			public:
				void SettingCluster(const GraphicDataSet& set, const JDrawHelper& helper);
				void SettingDebugging(const GraphicDataSet& set, const JDrawHelper& helper);
			}; 
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> forwardRootSignature;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> deferredGeometryRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> deferredShadingRootSignature;
		private:
			JOwnerPtr<JDx12GraphicShaderDataHolder> deferredShadingHolder[(uint)INNER_DEFERRED_SHADER_TYPE::COUNT];
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicBaseDataSet& baseDataSet)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			bool HasPreprocessing()const noexcept final;	//if true call BeginDraw before draw
			bool HasPostprocessing()const noexcept final;  //if true call EndDraw after draw
		public:
			void BindResource(const J_GRAPHIC_RENDERING_PROCESS process, const JGraphicBindSet* bindSet)final;
		private:
			//bind root and buffer resource(0...n)
			void BindForwardRootAndResource(JDx12FrameResource* dx12Frame, JDx12GraphicResourceManager* dx12Gm, ID3D12GraphicsCommandList* cmdList);
			void BindDeferredGeometryRootAndResource(JDx12FrameResource* dx12Frame, JDx12GraphicResourceManager* dx12Gm, ID3D12GraphicsCommandList* cmdList);
			void BindDeferredShadingRootAndResource(JDx12FrameResource* dx12Frame, JDx12GraphicResourceManager* dx12Gm, ID3D12GraphicsCommandList* cmdList);
		private:
			//Draw process
			void ClearResource(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper);
			void SwitchResourceStateForDrawing(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper);
			void SwitchResourceStateForDeferredShade(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper);
			void ReturnResourceState(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper);
			void BindRenderTarget(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper, const bool bindDs);
			void BindGBufferByRenderTarget(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper);
			void BindCommonCB(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper);
			void BindLightListResource(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper);	 
			void BindGBufferResource(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper);
			void BindViewPortAndRect(const GraphicDataSet& gSet, const ResourceDataSet& rSet);
			void DrawSceneGameObject(const GraphicDataSet& gSet, const JDrawHelper& helper);
			void DrawQuadForDeferredShading(const GraphicDataSet& gSet, const ResourceDataSet& rSet, const JDrawHelper& helper);	//for deferred rendering on single thread if multithread call DrawSceneShadeMultiThread
		public:
			//for multi thread
			//clear rtv dsv and set resource state
			void BeginDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)final;
			//for multi thread
			//set resource state
			void EndDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper)final;
		public:
			//Contain Draw Debug UI
			//DrawSceneRenderTarget에 경우 camera가 hdocc 지원시 해당 함수내에서 resource state를 predict로 변경한다.
			//추후에 Single thread draw 에서 Multi thread darw와 같이 BeginDraw. EndDraw와 같은 전처리 절차가 더 필요하지는 경우
			//Single Thread에서도 BeginDraw. EndDraw로 전처리를 처리하도록 수정하도록 한다.
			void DrawSceneRenderTarget(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)final;
			void DrawSceneRenderTargetMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)final;
			//only use forward rendering
			void DrawSceneDebugUI(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)final;
			void DrawSceneDebugUIMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)final;
			void DrawSceneShade(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)final;
			void DrawSceneShadeMultiThread(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)final;
		private:
			void DrawGameObject(const GraphicDataSet& set,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper& helper,
				const JDrawCondition& condition,
				const bool isDeferred);
		public:
			void RecompileShader(const JGraphicShaderCompileSet& dataSet)final;
			JOwnerPtr<JGraphicShaderDataHolderBase> CreateGraphicShader(const JGraphicShaderCompileSet& dataSet, JGraphicShaderInitData initData)final;
		private:
			void CompileShader(JDx12GraphicShaderDataHolder* holder, const JGraphicShaderInitData& initData);
			void StuffInputLayout(_Out_ std::vector<D3D12_INPUT_ELEMENT_DESC>& outInputLayout, const J_GRAPHIC_SHADER_VERTEX_LAYOUT vertexLayout);
			void StuffPso(const PsoBuildData& data, const JGraphicOption& option);
		private:
			void BuildForwardRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildDeferredGeometryRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildDeferredShadingRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void ReBuildRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
		private:
			void BuildDeferredShader(const JGraphicShaderCompileSet& dataSet);
		};
	}
}