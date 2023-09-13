#pragma once
#include"../../../Core/Math/JVector.h" 
#include"../../../Core/Pointer/JOwnerPtr.h"

namespace JinEngine
{
	class JMaterial;
	class JTexture;

	class JDefaultMaterialSetting
	{
	public:
		static void SetStandard(const JUserPtr<JMaterial>& mat);
		static void SetSky(const JUserPtr<JMaterial>& mat, const JUserPtr<JTexture>& skyTxt); 
		static void SetDebug(const JUserPtr<JMaterial>& mat, const bool isLine, const JVector4<float> color);
		static void SetAlbedoMapOnly(const JUserPtr<JMaterial>& mat, const JUserPtr<JTexture>& texture);
	};
}