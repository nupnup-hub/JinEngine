#pragma once 
#include"JFrameResourceEnum.h"
#include"../JGraphicConstants.h"
#include"../Device/JGraphicDeviceUser.h"
#include"../../Core/JCoreEssential.h"
#include"JFrameResource.h" 
#include"JObjectConstants.h" 
#include"JAnimationConstants.h" 
#include"JMaterialConstants.h" 
#include"JPassConstants.h" 
#include"JCameraConstants.h" 
#include"JLightConstants.h"  
#include"JBoundingObjectConstants.h"
#include"JOcclusionConstants.h"
#include"JShadowMapConstants.h"
#include"JDepthTestConstants.h"
#include"JSsaoConstants.h"
namespace JinEngine
{
	namespace Graphic
	{
		struct JGraphicInfo;
		class JGraphicDevice;
		class JGraphicBufferBase;
		class JFrameResource : public JGraphicDeviceUser
		{
		public:
			virtual void Intialize(JGraphicDevice* device, const JGraphicInfo& gInfo) = 0;
			virtual void Clear() = 0;
		public:
			virtual JGraphicBufferBase* GetGraphicBufferBase(const J_UPLOAD_FRAME_RESOURCE_TYPE type)const noexcept = 0;
			virtual uint GetElementCount(const J_UPLOAD_FRAME_RESOURCE_TYPE type)const noexcept = 0;
			uint GetLocalLightCount()const noexcept;
			virtual GraphicFence GetFenceValue()const noexcept = 0; 
		public:
			void CopyData(const J_UPLOAD_FRAME_RESOURCE_TYPE type, const int elementIndex, const void* data);
		public:
			virtual void ReBuild(JGraphicDevice* device, const J_UPLOAD_FRAME_RESOURCE_TYPE type, const uint newCount) = 0;
		};
	}
}