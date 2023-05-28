#pragma once 
#include<d3d12.h>
#include<wrl/client.h>
#include"../../Core/JDataType.h"
#include"../../Core/Func/Callable/JCallable.h"
#include"../Upload/JUploadBuffer.h" 
#include"../Upload/JUploadType.h" 

namespace JinEngine
{
	class JGameObject;
	class JLight;
	namespace Graphic
	{ 
		struct JGraphicInfo;
		struct JGraphicOption;
		struct JDrawHelper;
		struct JDrawCondition;
		class JFrameResource;
		class JGraphicResourceManager;
		class JHZBOccCulling;

		class JShadowMap
		{ 
		public:
			void Initialize(ID3D12Device* device, const JGraphicInfo& gInfo);
			void Clear(); 
		public:
			void DrawSceneShadowMap(ID3D12GraphicsCommandList* cmdList,
				JFrameResource* currFrame,
				JHZBOccCulling* occCulling,
				JGraphicResourceManager* graphicResource,
				const JGraphicOption& option,
				const JDrawHelper helper);
			void DrawShadowMapGameObject(ID3D12GraphicsCommandList* cmdList,
				JFrameResource* currFrame,
				JHZBOccCulling* occCulling,
				JGraphicResourceManager* graphicResource,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper helper,
				const JDrawCondition& condition);
		};
	}
}