#pragma once
#include"JOcclusionConstants.h" 
#include"JFrameConstantsSet.h"
#include"../../Core/Utility/JCommonUtility.h"

namespace JinEngine
{
	namespace Graphic
	{
		namespace CameraFrameLayer
		{ 
			static constexpr uint drawScene = 0;
			static constexpr uint depthTest= drawScene + 1;
			static constexpr uint hzb = depthTest + 1;
			static constexpr uint lightCulling = hzb + 1;
			static constexpr uint ssao = lightCulling + 1; 
			static constexpr uint setCount = ssao + 1;
		}

		struct JDrawSceneCameraConstants
		{
			//JMatrix4x4 view = JMatrix4x4::Identity();
			JMatrix4x4 invView = JMatrix4x4::Identity();
			//JMatrix4x4 proj = JMatrix4x4::Identity(); 
			//JMatrix4x4 invProj = JMatrix4x4::Identity();
			JMatrix4x4 viewProj = JMatrix4x4::Identity();
			JMatrix4x4 preViewProj = JMatrix4x4::Identity();
			//JMatrix4x4 invViewProj = JMatrix4x4::Identity(); 
			//JMatrix4x4 viewProjTex = JMatrix4x4::Identity();
			JVector2F renderTargetSize = JVector2F::Zero();
			JVector2F invRenderTargetSize = JVector2F::Zero();
			JVector2F uvToViewA = JVector2F::Zero();
			JVector2F uvToViewB = JVector2F::Zero();
			JVector3F eyePosW = JVector3F::Zero();
			float nearZ = 0.0f; 
			float farZ = 0.0f;
			uint csmLocalIndex = 0;
			int hasAoTexture = 0;
			uint cameraPad00 = 0;   
		};   
		struct JSsaoConstants
		{
			JMatrix4x4 camView;
			JMatrix4x4 camProj;
			float radius;
			float radius2;
			float bias;
			float sharpness;
			JVector2F camNearFar;
			JVector2F camRtSize;
			JVector2F camInvRtSize;
			JVector2F aoRtSize;
			JVector2F aoInvRtSize;
			JVector2F aoInvQuaterRtSize;
			JVector2F uvToViewA;
			JVector2F uvToViewB;
			JVector2F viewPortTopLeft;
			float radiusToScreen;
			float negInvR2;
			float tanBias;
			float smallScaleAOAmount;
			float largeScaleAOAmount;
			float camNearMulFar;
		};
		struct JLightCullingCameraConstants
		{
			JMatrix4x4 camView;
			JMatrix4x4 camProj;
			JVector2F camRenderTargetSize;
			JVector2F camInvRenderTargetSize;
			float camNearZ;
			float camFarZ;
			uint camPad00;
			uint camPad01;
		}; 

		struct JCameraConstantsSet : public JFrameConstantsSet<CameraFrameLayer::setCount>
		{ 
		public:
			JDrawSceneCameraConstants drawScene;
			JDepthTestConstants depthTest;
			JHzbOccComputeConstants hzb;
			JSsaoConstants ssao;
			JLightCullingCameraConstants lightCulling; 
		public:
			int hzbQueryCount = 0;
			int hzbQueryOffset = 0;  
		public:
			void SetUpdated(const uint layerIndex, const uint _frameIndex)
			{
				frameIndex[layerIndex] = _frameIndex;
				isUpdated[layerIndex] = true;
			}
		};
	}
}