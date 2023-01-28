#include"JDefaultMaterialSetting.h"
#include"JMaterial.h"

namespace JinEngine
{
	void JDefaultMaterialSetting::SetStandard(JMaterial* mat)
	{
		mat->SetShadow(true);
		mat->SetLight(true);
	}
	void JDefaultMaterialSetting::SetSky(JMaterial* mat, JTexture* texture)
	{
		mat->SetSkyMaterial(true);
		mat->SetNonCulling(true);
		mat->SetDepthCompareFunc(J_SHADER_DEPTH_COMPARISON_FUNC::LESS_EQUAL);
		mat->SetAlbedoMap(texture);
	}
	void JDefaultMaterialSetting::SetShadowMap(JMaterial* mat)
	{
		mat->SetShadowMapWrite(true);
		mat->SetAlphaClip(true);
	}
	void JDefaultMaterialSetting::SetDebug(JMaterial* mat, const bool isLine, const JVector4<float> color)
	{ 
		mat->SetDebugMaterial(true);
		if (isLine)
			mat->SetPrimitiveType(J_SHADER_PRIMITIVE_TYPE::LINE);
		mat->SetAlbedoColor(color.ConvertXMF()); 
	}
	void JDefaultMaterialSetting::SetAlbedoMapOnly(JMaterial* mat, JTexture* texture)
	{
		mat->SetAlbedoMap(texture);
		mat->SetAlbedoMapOnly(true);
	}
}