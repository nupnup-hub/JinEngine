#include"JDefaultMaterialSetting.h"
#include"JMaterial.h"
#include"JMaterialPrivate.h"

namespace JinEngine
{
	using UpdateShaderInterface = JMaterialPrivate::UpdateShaderInterface;

	void JDefaultMaterialSetting::SetStandard(JMaterial* mat)
	{
		UpdateShaderInterface::OffUpdateShaderTrigger(mat);
		mat->SetShadow(true);
		mat->SetLight(true);
		UpdateShaderInterface::OnUpdateShaderTrigger(mat);
		UpdateShaderInterface::UpdateShader(mat);
	}
	void JDefaultMaterialSetting::SetSky(JMaterial* mat, Core::JUserPtr<JTexture> texture)
	{
		UpdateShaderInterface::OffUpdateShaderTrigger(mat);
		mat->SetSkyMaterial(true);
		mat->SetNonCulling(true);
		mat->SetDepthCompareFunc(J_SHADER_DEPTH_COMPARISON_FUNC::LESS_EQUAL);
		mat->SetAlbedoMap(texture);
		UpdateShaderInterface::OnUpdateShaderTrigger(mat);
		UpdateShaderInterface::UpdateShader(mat);
	}
	void JDefaultMaterialSetting::SetShadowMap(JMaterial* mat)
	{
		UpdateShaderInterface::OffUpdateShaderTrigger(mat);
		mat->SetShadowMapWrite(true);
		mat->SetAlphaClip(true);
		UpdateShaderInterface::OnUpdateShaderTrigger(mat);
		UpdateShaderInterface::UpdateShader(mat);
	}
	void JDefaultMaterialSetting::SetDebug(JMaterial* mat, const bool isLine, const JVector4<float> color)
	{
		UpdateShaderInterface::OffUpdateShaderTrigger(mat);
		mat->SetDebugMaterial(true);
		if (isLine)
			mat->SetPrimitiveType(J_SHADER_PRIMITIVE_TYPE::LINE);
		mat->SetAlbedoColor(color.ConvertXMF()); 
		UpdateShaderInterface::OnUpdateShaderTrigger(mat);
		UpdateShaderInterface::UpdateShader(mat);
	}
	void JDefaultMaterialSetting::SetAlbedoMapOnly(JMaterial* mat, Core::JUserPtr<JTexture> texture)
	{
		UpdateShaderInterface::OffUpdateShaderTrigger(mat);
		mat->SetAlbedoMap(texture);
		mat->SetAlbedoMapOnly(true);
		UpdateShaderInterface::OnUpdateShaderTrigger(mat);
		UpdateShaderInterface::UpdateShader(mat);
	}
}