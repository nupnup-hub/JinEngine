#pragma once
#include"../JGraphicTaskInterface.h" 
#include"../JGraphicSubClassInterface.h"
#include"../../Core/Pointer/JOwnerPtr.h" 
#include<vector>

namespace JinEngine
{
	//struct JGraphicShaderData;
	class JGameObject;
	namespace Graphic
	{
		class JGraphicDevice;
		class JGraphicResourceManager;
		class JDrawHelper;
		struct JDrawCondition;

		class JDepthTest : public JGraphicTaskInterface, public JGraphicSubClassInterface
		{
		public:
			enum class TEST_TYPE
			{
				BOUNDING_TEST = 0,
				QUERY_TEST,
				QUERY_TEST_PASS,
				COUNT
			}; 
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) = 0;
			virtual void Clear() = 0;
		public:
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
		public:
			virtual void DrawSceneBoundingBox(const JGraphicDepthMapDrawSet* drawSet,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper& helper,
				const JDrawCondition& condition) = 0;
			virtual void DrawHdOcclusionQueryObject(const JGraphicDepthMapDrawSet* drawSet,
				const std::vector<JUserPtr<JGameObject>>& gameObject,
				const JDrawHelper& helper,
				const JDrawCondition& condition) = 0;
		public:
			virtual bool BindGraphicResource(const JGraphicBindSet* bindSet, const JDrawHelper& helper) = 0;
		};
	}
}