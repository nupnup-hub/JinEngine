#pragma once 
#include"../ShadowMap/JCsmOption.h"
#include"../JGraphicConstants.h"
#include"../../Core/JCoreEssential.h" 
#include"../../Core/Math/JMathHelper.h"
#include"../../Core/Math/JVector.h"
#include"../../Core/Math/JMatrix.h"
#include<DirectXMath.h> 

namespace JinEngine
{
	namespace Graphic
	{  
		struct JDirectionalLightConstants
		{
			JMatrix4x4 view = JMatrix4x4::Identity();
			JMatrix4x4 viewProj = JMatrix4x4::Identity();
			JMatrix4x4 shadowMapTransform = JMatrix4x4::Identity(); 
			JVector3F color = { 0.8f, 0.8f, 0.8f };
			int shadowMapIndex = 0;
			JVector3F direction = JVector3F::Down();
			int shadowMapType = 0;  //-1 is not 0 = norma, 1 = csm, 2 = cube
			JVector2F frustumSize;
			float frustumNear = 0;
			float frustumFar = 0;
			float penumbraScale = 0;
			float penumbraBlockerScale = 0;
			int csmDataIndex = 0;
			float shadowMapSize = 0;
			float shadowMapInvSize = 0;
			float tanAngle = 0;
			float bias = 0;
			uint dLightPad00 = 0;
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
			float innerConeAngle = 0;	//cos
			float outerConeAngle = 0;	//cos
			float penumbraScale = 0;
			float penumbraBlockerScale = 0;
			int shadowMapIndex = 0;
			uint hasShadowMap = 0;  
			float shadowMapSize = 0;
			float shadowMapInvSize = 0;
			float bias = 0;
			uint sLightPad00 = 0;
			uint sLightPad01 = 0;
			uint sLightPad02 = 0;
		}; 
		struct JRectLightConstants
		{ 
			JMatrix4x4 shadowMapTransform = JMatrix4x4::Identity();
			JVector3F origin;
			float power = 1.0f;
			JVector3F extents;
			float frustumNear = 0;
			JVector3F axis[3];
			JVector3F color = { 0.8f, 0.8f, 0.8f };
			float frustumFar = 0;
			uint isTwoSide = 0;
			int shadowMapIndex = 0;
			uint hasShadowMap = 0;
			int sourceTextureIndex = invalidIndex;
			int rectLightPad00 = 0;
			int rectLightPad01 = 0;
			int rectLightPad02 = 0;
		};
	}
}