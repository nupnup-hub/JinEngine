#pragma once
#include"../Device/JGraphicDeviceUser.h"
#include"../DataSet/JGraphicDataSet.h"
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
		struct JDrawHelper;
		struct JDrawCondition;

		class JDepthTest : public JGraphicDeviceUser
		{
		public:
			enum class TEST_TYPE
			{
				BOUNDING_TEST = 0,
				QUERY_TEST,
				COUNT
			}; 
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info) = 0;
			virtual void Clear() = 0;
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