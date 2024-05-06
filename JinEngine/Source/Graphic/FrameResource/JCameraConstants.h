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