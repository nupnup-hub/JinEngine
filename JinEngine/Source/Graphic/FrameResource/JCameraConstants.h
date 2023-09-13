#pragma once
#include"../../Core/Math/JVector.h"
#include"../../Core/Math/JMatrix.h"
#include"../../Core/JCoreEssential.h" 

namespace JinEngine
{
	namespace Graphic
	{
		struct JCameraConstants
		{
			JMatrix4x4 view = JMatrix4x4::Identity();
			JMatrix4x4 invView = JMatrix4x4::Identity();
			JMatrix4x4 proj = JMatrix4x4::Identity();
			JMatrix4x4 invProj = JMatrix4x4::Identity();
			JMatrix4x4 viewProj = JMatrix4x4::Identity();
			JMatrix4x4 invViewProj = JMatrix4x4::Identity(); 
			JVector2<float> renderTargetSize = { 0.0f, 0.0f };
			JVector2<float> invRenderTargetSize = { 0.0f, 0.0f };
			JVector3<float> eyePosW = { 0.0f, 0.0f, 0.0f };
			float nearZ = 0.0f;
			float farZ = 0.0f;
			uint csmLocalIndex = 0;
			uint cameraPad00 = 0;
			uint cameraPad01 = 0; 
		};
	}
}