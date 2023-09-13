#pragma once 
#include"JOccCulling.h"
#include"../../../Core/Pointer/JOwnerPtr.h"
#include"../../../Core/Math/JVector.h"

namespace JinEngine
{ 
	namespace Graphic
	{
		struct JGraphicInfo; 
		class JCullingInfo;
		class JHardwareOccCulling : public JOccCulling
		{
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM, const JGraphicInfo& info) = 0;
			virtual void Clear() = 0;
		public:
			virtual void NotifyBuildNewHdOccBuffer(JGraphicDevice* device, const size_t initCapacity, const JUserPtr<JCullingInfo>& cullingInfo) = 0;
			virtual void NotifyReBuildHdOccBuffer(JGraphicDevice* device, const size_t capacity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo) = 0;
			virtual void NotifyDestroyHdOccBuffer(JCullingInfo* cullingInfo) = 0;
		public:
			virtual void ExtractHDOcclusionCullingData(const JGraphicHdOccExtractSet* extractSet, const JDrawHelper& helper) = 0;
		};
	}
}
