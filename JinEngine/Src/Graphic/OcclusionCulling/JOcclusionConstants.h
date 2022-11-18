#pragma once
#include"../../Core/JDataType.h"
#include"../../Utility/JMathHelper.h"
#include<DirectXMath.h>

namespace JinEngine
{
	namespace Graphic
	{
		struct JBoundSphereConstants
		{
			DirectX::XMFLOAT3 center = DirectX::XMFLOAT3(0, 0, 0);
			float radius = 0;
		};
		struct JDepthMapInfoConstants
		{
			uint maxWidth = 0;
			uint maxHeight = 0;
			uint validMapCount = 0;
			uint samplePad00 = 0;
		};
		struct JOcclusionPassConstants
		{
			DirectX::XMFLOAT4X4 view = JMathHelper::Identity4x4();
			DirectX::XMFLOAT4X4 invView = JMathHelper::Identity4x4();
			DirectX::XMFLOAT4X4 proj = JMathHelper::Identity4x4();
			DirectX::XMFLOAT4X4 invProj = JMathHelper::Identity4x4();
			DirectX::XMFLOAT4X4 viewProj = JMathHelper::Identity4x4();
			DirectX::XMFLOAT4X4 invViewProj = JMathHelper::Identity4x4();
			DirectX::XMFLOAT2 renderTargetSize = { 0.0f, 0.0f };
			DirectX::XMFLOAT2 invRenderTargetSize = { 0.0f, 0.0f };
			DirectX::XMFLOAT3 eyePosW = { 0.0f, 0.0f, 0.0f };
			float nearZ = 0.0f;
			float farZ = 0.0f;
			uint validQueryCount = 0;
			uint cameraPad01 = 0;
			uint cameraPad02 = 0;
		};
	}
}