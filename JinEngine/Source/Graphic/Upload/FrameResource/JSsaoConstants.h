#pragma once
#include<DirectXMath.h>

namespace JinEngine
{
	namespace Graphic
	{
		struct JSsaoConstants
		{
			DirectX::XMFLOAT4X4 proj;
			DirectX::XMFLOAT4X4 invProj;
			DirectX::XMFLOAT4X4 projTex;
			DirectX::XMFLOAT4   offsetVectors[14];

			// For SsaoBlur.hlsl
			DirectX::XMFLOAT4 blurWeights[3];

			DirectX::XMFLOAT2 invRenderTargetSize = { 0.0f, 0.0f };

			// Coordinates given in view space.
			float occlusionRadius = 0.5f;
			float occlusionFadeStart = 0.2f;
			float occlusionFadeEnd = 2.0f;
			float surfaceEpsilon = 0.05f;
		};
	}
}