#pragma once 
#include"../../Core/Math/JVector.h"
#include"../../Core/Math/JMatrix.h"
#include"../Image/JImageProcessingConstants.h"
#include"../Image/JImageProcessingEnum.h"
namespace JinEngine
{
	namespace Graphic
	{
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
	}
}