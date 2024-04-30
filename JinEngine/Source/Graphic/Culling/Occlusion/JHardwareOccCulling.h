#pragma once 
#include"JOccCulling.h"
#include"../../JGraphicSubClassInterface.h"
#include"../../../Core/Pointer/JOwnerPtr.h"
#include"../../../Core/Math/JVector.h"

namespace JinEngine
{ 
	namespace Graphic
	{
		struct JGraphicInfo; 
		class JCullingInfo;
		/**
		* Hd IsOccluder trigger에 관계없이 Object에 Occlude 여부를 검사한다
		*/
		class JHardwareOccCulling : public JOccCulling, public JGraphicSubClassInterface
		{
		public:
			virtual void Initialize(JGraphicDevice* device, JGraphicResourceManager* gM) = 0;
			virtual void Clear() = 0;
		public:
			bool IsSupported(const J_GRAPHIC_TASK_TYPE taskType)const noexcept final;
		public:
			virtual void NotifyBuildNewHdOccBuffer(JGraphicDevice* device, const size_t initCapacity, const JUserPtr<JCullingInfo>& cullingInfo) = 0;
			virtual void NotifyReBuildHdOccBuffer(JGraphicDevice* device, const size_t capacity, const std::vector<JUserPtr<JCullingInfo>>& cullingInfo) = 0;
			virtual void NotifyDestroyHdOccBuffer(JCullingInfo* cullingInfo) = 0;
		public:
			virtual void ExtractHDOcclusionCullingData(const JGraphicHdOccExtractSet* extractSet, const JDrawHelper& helper) = 0;
		};
	}
}
