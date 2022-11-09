#pragma once
#include<DirectXMath.h> 
#include"../../../Utility/JMathHelper.h"

namespace JinEngine
{
	struct JLightStruct
	{
		DirectX::XMFLOAT3 strength = {0.8f, 0.8f, 0.8f};
		float falloffStart = 1.0f;                          // point/spot light only
		DirectX::XMFLOAT3 direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
		float falloffEnd = 10.0f;                           // point/spot light only
		DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };  // point/spot light only
		float spotPower = 0;                            // spot light only
	};

	struct JShadowMapStruct
	{
		DirectX::XMFLOAT4X4 shadowTransform = JMathHelper::Identity4x4();
		uint shadowMapIndex = 0;
		uint shadowPad00 = 0;
		uint shadowPad01 = 0;
		uint shadowPad02 = 0;
	};

	struct JDirectionalLight
	{
		DirectX::XMFLOAT3 strength = { 0.8f, 0.8f, 0.8f };
		uint dLightPad00 = 0;
		DirectX::XMFLOAT3 direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
		uint dLightPad01 = 0;
	}; 
	//수정필요
	struct JSMDirectionalLight
	{
		JDirectionalLight dLight;
		JShadowMapStruct shadow;
	};
	struct JPointLight
	{
		DirectX::XMFLOAT3 strength = { 0.8f, 0.8f, 0.8f };
		float falloffStart = 1.0f;                       
		DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f }; 
		float falloffEnd = 10.0f;
	};
	struct JSMPointLight
	{
		JPointLight pLight;
		JShadowMapStruct shadow;
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
	struct JSMSpotLight
	{
		JSpotLight sLight;
		JShadowMapStruct shadow;
	};
	namespace Constant
	{
		static constexpr int maxLight = 16;
	}
}