#pragma once
#include"../../Math/JVector.h"

namespace JinEngine
{
	namespace Core
	{
		//obj, fbx등 널리 사용되는 공통 parameter 정의
		class JMaterialParameter
		{ 
		public:
			JVector4F albedoColor = InitAlbedoColor();
			float metallic = InitMetalic();
			float roughness = InitRoughness();
			float specularFactor = InitSpecularFactor();
		public:
			static inline JVector4F InitAlbedoColor() { return  { 1.0f, 1.0f, 1.0f, 1.0f }; }
			static inline float InitMetalic() { return  0.25f; }
			static inline float InitRoughness() { return  0.75f; }
			static inline float InitSpecularFactor() { return  0.5f; }
		};
	}
}