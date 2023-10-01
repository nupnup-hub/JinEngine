#pragma once
#include"../JShadowMap.h"
#include"../JCsmOption.h"
#include"../../Shader/Dx/JDx12ShaderDataHolder.h"
#include"../../../Core/Geometry/Mesh/JMeshType.h"
#include"../../../Object/Component/Light/JLightType.h"
#include<d3d12.h>
#include<wrl/client.h>

namespace JinEngine
{
	class JGameObject;
	namespace Graphic
	{
		struct JDrawCondition;
		class JDx12FrameResource;
		class JDx12CullingManager;
		class JDx12GraphicResourceManager;
		class JDx12GraphicShaderDataHolder;
		class JDx12GraphicDevice;
		class JDx12ShadowMap : public JShadowMap
		{ 
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
			std::unique_ptr<JDx12GraphicShaderDataHolder> normalShadowShaderData[(uint)Core::J_MESHGEOMETRY_TYPE::COUNT];
			std::unique_ptr<JDx12GraphicShaderDataHolder> cubeShadowShaderData[(uint)Core::J_MESHGEOMETRY_TYPE::COUNT];
			std::unique_ptr<JDx12GraphicShaderDataHolder> csmShaderData[(uint)Core::J_MESHGEOMETRY_TYPE::COUNT][JCsmOption::maxCountOfSplit];
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info)final;
			void Clear()final;
		public:
			J_GRAPHIC_DEVICE_TYPE GetDeviceType()const noexcept final;
		private:
			JDx12GraphicShaderDataHolder* GetShaderDataHolder(const JDrawHelper& helper, const Core::J_MESHGEOMETRY_TYPE mType);
		public:
			void BindResource(const JGraphicBindSet* bindSet)final;
		public:
			bool HasPreprocessing()const noexcept final;	//if true call BeginDraw before draw
			bool HasPostprocessing()const noexcept final;  //if true call EndDraw after draw
		public:
			//for multi thread
			//clear rtv dsv and set resource state
			void BeginDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper) final;
			//for multi thread
			//set resource state
			void EndDraw(const JGraphicBindSet* bindSet, const JDrawHelper& helper) final;
		public:
			//Contain Draw Debug UI
			void DrawSceneShadowMap(const JGraphicShadowMapDrawSet* shadowDrawSet, const JDrawHelper& helper)final;
			void DrawSceneShadowMapMultiThread(const JGraphicShadowMapDrawSet* shadowDrawSet, const JDrawHelper& helper)final;
		public:
			void DrawShadowMapGameObject(ID3D12GraphicsCommandList* cmdList,
				JDx12FrameResource* dx12Frame,
				JDx12GraphicResourceManager* dx12Gm,
				JDx12CullingManager* dx12Cm,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper& helper,
				const JDrawCondition& condition,
				const uint dataIndex);
		private:
			void BindLightFrameResource(ID3D12GraphicsCommandList* cmdList, JDx12FrameResource* dx12Frame, const JDrawHelper& helper, const int offset);
		private:
			void BuildRootSignature(ID3D12Device* device);
			void BuildPso(ID3D12Device* device,
				const DXGI_FORMAT dsvFormat,
				const Core::J_MESHGEOMETRY_TYPE meshType,
				const J_SHADOW_MAP_TYPE smType,
				const std::vector<JMacroSet>& macroSet,
				_Out_ JDx12GraphicShaderDataHolder& data);
		};
	}
}
