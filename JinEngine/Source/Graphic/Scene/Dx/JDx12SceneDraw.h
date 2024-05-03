#pragma once
#include"../JSceneDraw.h"
#include"../../DataSet/Dx/JDx12GraphicDataSet.h"
#include"../../GraphicResource/Dx/JDx12GraphicResourceManager.h"
#include"../../GraphicResource/JGraphicResourceInterface.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h" 
#include<d3d12.h>    
#include<vector>
#include<wrl/client.h>

namespace JinEngine
{
	namespace Graphic
	{
		struct JDrawCondition;
		class JDx12CommandContext;
		class JDx12FrameResource;
		class JDx12CullingManager;
		class JDx12GraphicResourceManager;
		class JDx12GraphicDevice;
		class JDx12SceneDraw final : public JSceneDraw
		{
		private:
			using JDx12GraphicShaderDataHolder = JDx12GraphicShaderDataHolder<(uint)J_GRAPHIC_SHADER_EXTRA_FUNCTION::COUNT>;
			using JDx12ComputeShaderDataHolder = JDx12ComputeShaderDataHolder<1>;
		public:
			enum INNER_DEFERRED_SHADER_TYPE
			{
				INNER_DEFERRED_SHADER_EXCEPT_ALL = 0,
				INNER_DEFERRED_SHADER_LIGHT_CULLING = 1 << 0,
				INNER_DEFERRED_SHADER_GI = 1 << 1,
				INNER_DEFERRED_SHADER_INCLUDE_ALL = INNER_DEFERRED_SHADER_LIGHT_CULLING | INNER_DEFERRED_SHADER_GI,
				INNER_DEFERRED_SHADER_COUNT = 1 << 2
			};
			enum class INNER_STEP
			{
				SETTING,
				FORWARD,
				DEFERRED_GEO,
				DEFERRED_SHADE
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
				JGraphicShaderCondition condition;
			public:
				INNER_DEFERRED_SHADER_TYPE deferredType;
			};
			struct ResourceDataSet
			{
			public:
				//Drawing data
				uint sceneCBIndex = 0;
				uint camCBIndex = 0;
				int camFrameIndex = invalidIndex;
				int sceneFrameIndex = invalidIndex;
			public:
				Graphic::JGraphicResourceUserInterface gRInterface;
			public:
				JDx12GraphicResourceComputeSet rtSet;
				JDx12GraphicResourceComputeSet dsSet;
			public:
				//for rt spatial-temporal process
				JDx12GraphicResourceComputeSet preRsSet;
				JDx12GraphicResourceComputeSet preDsSet;
				JDx12GraphicResourceComputeSet preNormalSet;
				JDx12GraphicResourceComputeSet preVelocitySet;
			public:
				JDx12GraphicResourceComputeSet gBufferSet[Constants::gBufferLayerCount];
			public:
				JDx12GraphicResourceComputeSet velocitySet;
				JDx12GraphicResourceComputeSet aoSet;
			public:
				JDx12GraphicResourceComputeSet debugSet;
			public:
				JDx12GraphicResourceComputeSet clusterOffsetSet;
				JDx12GraphicResourceComputeSet clusterListSet; 
			public:
				JDx12GraphicResourceComputeSet giColorSet; 
			public:
				bool canUseAo = false;
				bool canUseLightCulling = false;
				bool canUseLightCluster = false;
				bool canUseGi = false;
			public:
				ResourceDataSet(JDx12CommandContext* context, const JDrawHelper& helper);
			public:
				void SettingCluster(JDx12CommandContext* context, const JDrawHelper& helper);
				void SettingGi(JDx12CommandContext* context, const JDrawHelper& helper);
				void SettingDebugging(JDx12CommandContext* context, const JDrawHelper& helper);
			public:
				INNER_DEFERRED_SHADER_TYPE GetDeferredType()const noexcept;
			public:
				bool IsValid()const noexcept;
			};
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> forwardRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> deferredGeometryRootSignature;
			Microsoft::WRL::ComPtr<ID3D12RootSignature> deferredShadingRootSignature;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> velocityRootsignature;
			std::unique_ptr<JDx12ComputeShaderDataHolder> velocityShader;
		private:
			JOwnerPtr<JDx12GraphicShaderDataHolder> deferredShadingHolder[INNER_DEFERRED_SHADER_COUNT];
		public:
			~JDx12SceneDraw();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		public:
			bool HasPreprocessing()const noexcept final;	//if true call BeginDraw before draw
			bool HasPostprocessing()const noexcept final;  //if true call EndDraw after draw
		private:
			bool HasDependency(const JGraphicInfo::TYPE type)const noexcept final;
			bool HasDependency(const JGraphicOption::TYPE type)const noexcept final;
		private:
			void NotifyGraphicInfoChanged(const JGraphicInfoChangedSet& set)final;
			void NotifyGraphicOptionChanged(const JGraphicOptionChangedSet& set)final;
		public:
			void BindResource(const J_GRAPHIC_RENDERING_PROCESS process, const JGraphicBindSet* bindSet)final;
		private:
			//bind root and buffer resource(0...n)
			void BindForwardRootAndResource(JDx12CommandContext* context);
			void BindDeferredGeometryRootAndResource(JDx12CommandContext* context);
			void BindDeferredShadingRootAndResource(JDx12CommandContext* context);
		private:
			//Draw process
			void ClearResource(JDx12CommandContext* context, ResourceDataSet& rSet, const JDrawHelper& helper);
			void SwitchResourceStateForDrawing(JDx12CommandContext* context, ResourceDataSet& rSet, const JDrawHelper& helper);
			void SwitchResourceStateForDeferredShade(JDx12CommandContext* context, ResourceDataSet& rSet, const JDrawHelper& helper);
			void ReturnResourceState(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper);
			void BindRenderTarget(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper, const bool bindDs);
			void BindGBufferByRenderTarget(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper);
			void BindCommonCB(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper);
			void BindAoResource(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper);
			void BindLightListResource(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper);
			void BindGiResource(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper);
			void BindGBufferResource(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper);
			void BindViewPortAndRect(JDx12CommandContext* context, const ResourceDataSet& rSet);
			void DrawSceneGameObject(JDx12CommandContext* context, const JDrawHelper& helper);
			void DrawFullScreenGeometry(JDx12CommandContext* context, const ResourceDataSet& rSet, const JDrawHelper& helper);	//for deferred rendering on single thread if multithread call DrawSceneShadeMultiThread	
			void CopyPreTemporalResource(JDx12CommandContext* context, ResourceDataSet& rSet, const JDrawHelper& helper);
			void ComputeVelocity(JDx12CommandContext* context, const ResourceDataSet& set, const JDrawHelper& helper);
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
		public:
			void ComputeSceneDependencyTemporalResource(const JGraphicSceneDrawSet* drawSet, const JDrawHelper& helper)final;
		private:
			void DrawGameObject(JDx12CommandContext* context,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper& helper,
				const JDrawCondition& condition,
				const bool isDeferred);
		public:
			void RecompileShader(const JGraphicShaderCompileSet& dataSet)final;
			JOwnerPtr<JShaderDataHolder> CreateShader(const JGraphicShaderCompileSet& dataSet, JGraphicShaderInitData initData)final;
		private:
			void CompileShader(JDx12GraphicShaderDataHolder* holder, const JGraphicShaderInitData& initData);
			void StuffInputLayout(_Out_ std::vector<D3D12_INPUT_ELEMENT_DESC>& outInputLayout, const J_GRAPHIC_SHADER_VERTEX_LAYOUT vertexLayout);
			void StuffPso(const PsoBuildData& data, const JGraphicOption& option);
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM);
			void BuildForwardRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildDeferredGeometryRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildDeferredShadingRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildVelocityRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void ReBuildRootSignature(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
			void BuildDeferredShader(const JGraphicShaderCompileSet& dataSet);
			void BuildVelocityShader(ID3D12Device* device, const JGraphicInfo& info, const JGraphicOption& option);
		private:
			void ClearResource();
		};
	}
}