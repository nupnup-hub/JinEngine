#pragma once
#include"../JGraphicResourceUserInterface.h"
#include"../Upload/JUploadBuffer.h"
#include<d3d12.h>
#include<memory>
#include<wrl/client.h> 

namespace JinEngine
{
	namespace Graphic
	{
		struct JGraphicDrawHelper;
		struct JGraphicOption;
		struct JShadowMapConstants;

		class JShadowMap : public JGraphicResourceUserInterface
		{
		private:
			Microsoft::WRL::ComPtr<ID3D12RootSignature> mRootSignature;
		private:
			std::unique_ptr<JUploadBuffer<JShadowMapConstants>> shadowCalCB = nullptr;
		public:
			void DrawSceneShadowMap(ID3D12GraphicsCommandList* commandList,
				ID3D12Resource* ds,
				const CD3DX12_CPU_DESCRIPTOR_HANDLE dsv, 
				const JGraphicDrawHelper& helper,
				const JGraphicOption& optionr);
		};
	}
}