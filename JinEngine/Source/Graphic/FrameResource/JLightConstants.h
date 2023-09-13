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
			JMatrix4x4 shadowMapTransform = JMatrix4x4::Identity();
			JVector3<float> color = { 0.8f, 0.8f, 0.8f };
			int shadowMapIndex = 0;
			JVector3<float> direction = JVector3<float>::Down();
			int shadowMapType = 0;  //-1 is not 0 = norma, 1 = csm, 2 = cube
			int csmDataIndex = 0;
			uint dLightPad00 = 0;
			uint dLightPad01 = 0;
			uint dLightPad02 = 0;
		};
		struct JCsmConstants
		{
			JVector4<float> scale[JCsmOption::maxCountOfSplit];        //rate (cascade area) / (scene area)
			JVector4<float> posOffset[JCsmOption::maxCountOfSplit];
			float mapMinBorder = 0;     //default value is 0
			float mapMaxBorder = 1;     //default value is 1
			uint count = 0; 
			uint csmLightPad00;
		};
		struct JPointLightConstants
		{
			JVector3<float> color = { 0.8f, 0.8f, 0.8f };
			float falloffStart = 1.0f;
			JVector3<float> position = { 0, 0, 0 };
			float falloffEnd = 100.0f;
			float nearPlane = 0.1f;        //default is 0.1f
			float farPlane = falloffEnd;          //default is falloffEnd
			uint shadowMapIndex = 0;
			uint hasShadowMap = 0;
		};
		struct JSpotLightConstants
		{
			JMatrix4x4 shadowMapTransform = JMatrix4x4::Identity();
			JVector3<float> color = { 0.8f, 0.8f, 0.8f };
			float falloffStart = 1.0f;
			JVector3<float> position;
			float falloffEnd = 100.0f;
			JVector3<float> direction;
			float power = 1.0f;
			float angle = 15.0f * JMathHelper::DegToRad;    //rad
			uint shadowMapIndex = 0;
			uint hasShadowMap = 0;
			uint sLightPad00 = 0;
		}; 
	}
}