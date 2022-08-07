#pragma once
#include"../../Utility/JMathHelper.h" 
#include"../../Core/JDataType.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JShadowMapConstants
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
			uint shadowPad00 = 0;
			float nearZ = 0.0f;
			float farZ = 0.0f;
			uint shadowPad01 = 0;
			uint shadowPad02 = 0;
		};
	}
}