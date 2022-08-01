#pragma once
#include<DirectXMath.h>
#include"JShadowStruct.h"

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

	struct JDirectionalLight
	{
		DirectX::XMFLOAT3 strength = { 0.8f, 0.8f, 0.8f };
		uint dLightPad00 = 0;
		DirectX::XMFLOAT3 direction = { 0.0f, -1.0f, 0.0f };// directional/spot light only
		uint dLightPad01 = 0;
	}; 
	//수정필요
	struct S_DirectionalLight
	{
		JDirectionalLight dLight;
		JShadow shadow;
	};
	struct JPointLight
	{
		DirectX::XMFLOAT3 strength = { 0.8f, 0.8f, 0.8f };
		float falloffStart = 1.0f;                       
		DirectX::XMFLOAT3 position = { 0.0f, 0.0f, 0.0f }; 
		float falloffEnd = 10.0f;
	};
	struct S_PointLight
	{
		JPointLight pLight;
		JShadow shadow;
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
	struct S_SpotLight
	{
		JSpotLight sLight;
		JShadow shadow;
	};
	static constexpr int maxLight = 16; 
}