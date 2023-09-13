#pragma once 
#include"../../Core/Math/JVector.h"
#include"../../Core/Math/JMatrix.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JSsaoConstants
		{
			JMatrix4x4 proj;
			JMatrix4x4 invProj;
			JMatrix4x4 projTex;
			JVector4<float>   offsetVectors[14];

			// For SsaoBlur.hlsl
			JVector4<float> blurWeights[3];
			JVector2<float> invRenderTargetSize = { 0.0f, 0.0f };

			// Coordinates given in view space.
			float occlusionRadius = 0.5f;
			float occlusionFadeStart = 0.2f;
			float occlusionFadeEnd = 2.0f;
			float surfaceEpsilon = 0.05f;
		};
	}
}