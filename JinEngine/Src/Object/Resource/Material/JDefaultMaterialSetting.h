#pragma once
#include"../../../Utility/JVector.h"

namespace JinEngine
{
	class JMaterial;
	class JTexture;

	class JDefaultMaterialSetting
	{
	public:
		static void SetStandard(JMaterial* mat);
		static void SetSky(JMaterial* mat, JTexture* skyTxt);
		static void SetShadowMap(JMaterial* mat);
		static void SetDebug(JMaterial* mat, const bool isLine, const JVector4<float> color);
		static void SetAlbedoMapOnly(JMaterial* mat, JTexture* texture);
	};
}