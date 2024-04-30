#pragma once 
#include"JOcclusionConstants.h" 
#include"JFrameConstantsSet.h"
#include"../ShadowMap/JCsmOption.h"
#include"../JGraphicConstants.h" 
#include"../../Core/Math/JMathHelper.h" 
#include"../../Core/Utility/JCommonUtility.h"
#include<DirectXMath.h> 

namespace JinEngine
{
	namespace Graphic
	{  
		namespace LightFrameLayer
		{
			static constexpr uint light = 0; 
			static constexpr uint csm = light + 1;
			static constexpr uint shadowMap = csm + 1;
			static constexpr uint shadowMapArray = shadowMap + 1;
			static constexpr uint shadowMapCube = shadowMapArray + 1;
			static constexpr uint depthTest = shadowMapCube + 1;
			static constexpr uint hzb = depthTest + 1; 
			static constexpr uint setCount = hzb + 1;
		}

		struct JDirectionalLightConstants
		{
			JMatrix4x4 view = JMatrix4x4::Identity();
			JMatrix4x4 viewProj = JMatrix4x4::Identity();
			JMatrix4x4 shadowMapTransform = JMatrix4x4::Identity(); 
			JVector3F color = { 0.8f, 0.8f, 0.8f };
			float power = 0;
			JVector3F direction = JVector3F::Down();
			float frustumNear = 0;
			JVector2F frustumSize;
			float frustumFar = 0;
			float penumbraScale = 0;
			float penumbraBlockerScale = 0;
			int csmDataIndex = 0;
			int shadowMapIndex = 0; 
			int shadowMapType = 0;  //-1 is not 0 = norma, 1 = csm, 2 = cube
			float shadowMapSize = 0;
			float shadowMapInvSize = 0;	 
			float tanAngle = 0;
			float bias = 0;
		}; 
		struct JCsmConstants
		{ 
			JVector4<float> scale[JCsmOption::maxCountOfSplit];        //rate (cascade area) / (scene area)
			JVector4<float> posOffset[JCsmOption::maxCountOfSplit];
			//JVector2<float> frustumSize[JCsmOption::maxCountOfSplit];
			float frustumNear[JCsmOption::maxCountOfSplit];
			float frustumFar[JCsmOption::maxCountOfSplit]; 
			float mapMinBorder = 0;     //default value is 0
			float mapMaxBorder = 1;     //default value is 1
			float levelBlendRate = 0;
			uint count = 0;  
		};
		struct JPointLightConstants
		{
			JMatrix4x4 shadowMapTransform[Constants::cubeMapPlaneCount];
			JVector3F midPosition;
			float power = 1.0f;
			JVector3F sidePosition[2];
			float radius = 0;
			float frustumNear = 0;
			JVector3F color = { 0.8f, 0.8f, 0.8f };
			float frustumFar = 0;
			float penumbraScale = 0;
			float penumbraBlockerScale = 0;
			int shadowMapIndex = 0;
			uint hasShadowMap = 0; 
			float shadowMapSize = 0;
			float shadowMapInvSize = 0;
			float bias = 0; 
			uint pLightPad00 = 0;
		};
		struct JSpotLightConstants
		{
			JMatrix4x4 shadowMapTransform = JMatrix4x4::Identity();
			JVector3F color = { 0.8f, 0.8f, 0.8f };
			float power = 1.0f;
			JVector3F position;
			float frustumNear = 0;
			JVector3F direction;
			float frustumFar = 0;
			float innerConeCosAngle = 0;	//cos
			float outerConeCosAngle = 0;	//cos
			float outerConeAngle = 0;
			float penumbraScale = 0;
			float penumbraBlockerScale = 0;
			int shadowMapIndex = 0;
			uint hasShadowMap = 0;  
			float shadowMapSize = 0;
			float shadowMapInvSize = 0;
			float bias = 0;
			uint sLightPad00 = 0;
			uint sLightPad01 = 0; 
		}; 
		struct JRectLightConstants
		{ 
			JMatrix4x4 shadowMapTransform = JMatrix4x4::Identity();
			JVector3F origin;
			float power = 1.0f; 
			JVector3F axis[3];
			JVector3F direction;
			JVector3F color = { 0.8f, 0.8f, 0.8f };
			float frustumNear;
			JVector2F extents;
			float frustumFar;
			float barndoorLength;
			float barndoorCosAngle;
			int shadowMapIndex = 0;
			uint hasShadowMap = 0;
			int sourceTextureIndex = invalidIndex;
			int ltcMatTextureIndex = 0;
			int ltcAmpTextureIndex = 0;
			int rectLightPad00 = 0;
			int rectLightPad01 = 0;
		};  
		
		struct JShadowMapDrawConstants
		{
			JMatrix4x4 shadowMapTransform;
		};
		struct JShadowMapArrayDrawConstants
		{
			JMatrix4x4 shadowMapTransform[JCsmOption::maxCountOfSplit];
		};
		struct JShadowMapCubeDrawConstants
		{
			JMatrix4x4 shadowMapTransform[Constants::cubeMapPlaneCount];
		};

		struct JLightConstantsSet : public JFrameConstantsSet<LightFrameLayer::setCount>
		{
		public:
			JDirectionalLightConstants directionalLight;
			JPointLightConstants pointLight;
			JSpotLightConstants spotLight;
			JRectLightConstants rectLight;
			std::vector<JCsmConstants> csm;
			JShadowMapDrawConstants shadowMap;
			std::vector<JShadowMapArrayDrawConstants> shadowMapArray;
			JShadowMapCubeDrawConstants shadowMapCube;
			JDepthTestConstants depthTest;
			JHzbOccComputeConstants hzb;    
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