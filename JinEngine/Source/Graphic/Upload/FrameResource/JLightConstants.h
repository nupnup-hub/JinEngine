#pragma once 
#include"../../../Core/JDataType.h" 
#include"../../../Utility/JMathHelper.h"
#include<DirectXMath.h> 

namespace JinEngine
{
	namespace Graphic
	{ 
		struct JLightIndexConstants
		{
			uint litStIndex = 0;
			uint litEdIndex = 0;
			uint shadwMapStIndex = 0;
			uint shadowMapEdIndex = 0;
		};
		struct JLightConstants
		{
			DirectX::XMFLOAT3 strength = { 0.8f, 0.8f, 0.8f };
			float falloffStart = 1.0f;                          // point/spot light only
			DirectX::XMFLOAT3 direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
			float falloffEnd = 10.0f;                           // point/spot light only
			DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
			float spotPower = 0;                            // spot light only
			uint lightType = 0;								// 0 = dirctional 1 = point 2 = spot
			uint lightPad00 = 0;
			uint lightPad01 = 0;
			uint lightPad02 = 0;
		};
		struct JShadowMapLightConstants
		{
			DirectX::XMFLOAT4X4 shadowTransform = JMathHelper::Identity4x4();
			DirectX::XMFLOAT3 strength = { 0.8f, 0.8f, 0.8f };
			float falloffStart = 1.0f;                          // point/spot light only
			DirectX::XMFLOAT3 direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
			float falloffEnd = 10.0f;                           // point/spot light only
			DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
			float spotPower = 0;								// spot light only
			uint lightType = 0;								// 0 = dirctional 1 = point 2 = spot
			uint shadowMapIndex = 0;
			uint shadowPad00 = 0;
			uint shadowPad01 = 0; 
		};
		struct JDirectionalLight
		{
			DirectX::XMFLOAT3 strength = { 0.8f, 0.8f, 0.8f };
			uint dLightPad00 = 0;
			DirectX::XMFLOAT3 direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
			uint dLightPad01 = 0;
		};
		struct JPointLight
		{
			DirectX::XMFLOAT3 strength = { 0.8f, 0.8f, 0.8f };
			float falloffStart = 1.0f;
			DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
			float falloffEnd = 10.0f;
		};
		struct JSpotLight
		{
			DirectX::XMFLOAT3 strength = { 0.8f, 0.8f, 0.8f };
			float falloffStart = 1.0f;
			DirectX::XMFLOAT3 direction = { 0.0f, -1.0f, 0.0f };
			float falloffEnd = 10.0f;
			DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
			float spotPower = 0;
		}; 
	}
}