#pragma once 
#include"../ShadowMap/JCsmOption.h"
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
			JVector3<float> color = { 0.8f, 0.8f, 0.8f };
			int shadowMapIndex = 0;
			JVector3<float> direction = JVector3<float>::Down();
			int shadowMapType = 0;  //-1 is not 0 = norma, 1 = csm, 2 = cube
			JVector2F frustumSize;
			float frustumNear = 0;
			float frustumFar = 0;
			float punumbraScale = 0;
			float penumbraNearPlane = 0;
			int csmDataIndex = 0;
			uint dLightPad00 = 0;
		};
		struct JCsmConstants
		{ 
			JVector4<float> scale[JCsmOption::maxCountOfSplit];        //rate (cascade area) / (scene area)
			JVector4<float> posOffset[JCsmOption::maxCountOfSplit];
			JVector2<float> frustumSize[JCsmOption::maxCountOfSplit];
			float frustumNear[JCsmOption::maxCountOfSplit];
			float frustumFar[JCsmOption::maxCountOfSplit]; 
			float mapMinBorder = 0;     //default value is 0
			float mapMaxBorder = 1;     //default value is 1
			float levelBlendRate = 0;
			uint count = 0;  
		};
		struct JPointLightConstants
		{
			JVector3<float> color = { 0.8f, 0.8f, 0.8f };
			float power = 1.0f;
			JVector3<float> position = { 0, 0, 0 };
			float range = 100.0f; 
			float radius = 0.0f;
			int shadowMapIndex = 0;
			uint hasShadowMap = 0;
			uint pLightPad00 = 0;
		};
		struct JSpotLightConstants
		{
			JMatrix4x4 shadowMapTransform = JMatrix4x4::Identity();
			JVector3<float> color = { 0.8f, 0.8f, 0.8f };
			float power = 1.0f;
			JVector3<float> position;
			float range = 1.0f;
			JVector3<float> direction;
			float innerConeAngle = 0;	//cos
			float outerConeAngle = 0;	//cos
			int shadowMapIndex = 0;
			uint hasShadowMap = 0; 
			uint sLightPad00 = 0;;
		}; 
	}
}