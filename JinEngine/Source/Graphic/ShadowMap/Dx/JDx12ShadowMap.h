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
		class JDx12GraphicDevice;
		class JDx12CommandContext;

		class JDx12ShadowMap : public JShadowMap
		{ 
		private:
			using JDx12GraphicShaderDataHolder = JDx12GraphicShaderDataHolder<1>;
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
			std::unique_ptr<JDx12GraphicShaderDataHolder> normalShadowShaderData[(uint)Core::J_MESHGEOMETRY_TYPE::COUNT];
			std::unique_ptr<JDx12GraphicShaderDataHolder> cubeShadowShaderData[(uint)Core::J_MESHGEOMETRY_TYPE::COUNT];
			std::unique_ptr<JDx12GraphicShaderDataHolder> csmShaderData[JCsmOption::maxCountOfSplit][(uint)Core::J_MESHGEOMETRY_TYPE::COUNT];
		public:
			~JDx12ShadowMap();
		public:
			void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM)final;
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
		private:
			void DrawShadowMapGameObject(JDx12CommandContext* context,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper& helper,
				const JDrawCondition& condition,
				const uint dataIndex);
		private:
			void BindLightFrameResource(JDx12CommandContext* context, const JDrawHelper& helper, const int offset);
		private:
			void BuildResource(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info);
			void BuildRootSignature(ID3D12Device* device);
			void BuildPso(ID3D12Device* device,
				const DXGI_FORMAT dsvFormat,
				const J_SHADOW_MAP_TYPE smType, 
				const Core::J_MESHGEOMETRY_TYPE meshType,
				const std::vector<JMacroSet>& macroSet,
				_Out_ JDx12GraphicShaderDataHolder& data);
		private:
			void ClearResource();
		};
	}
}
