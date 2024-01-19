#pragma once 
#include"../../Core/Math/JVector.h"
#include"../../Core/Math/JMatrix.h"
#include"../../Core/JCoreEssential.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JMaterialConstants
		{
			JVector4<float> albedoColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			// Used in texture mapping.
			JMatrix4x4 matTransform = JMatrix4x4::Identity();
			float metalic = 0.75f;
			float roughness = 0.3f;
			/*
			* srv heap index
			1 = missing
			*/
			uint albedoMapIndex = 1;
			uint normalMapIndex = 1;
			uint heightMapIndex = 1;
			uint metallicMapIndex = 1;
			uint roughnessMapIndex = 1;
			uint ambientOcclusionMapIndex = 1; 
		};
	}
}