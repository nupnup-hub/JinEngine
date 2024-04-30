#include"JDefaultMaterialSetting.h"
#include"JMaterial.h"
#include"JMaterialPrivate.h"

namespace JinEngine
{
	using UpdateShaderInterface = JMaterialPrivate::UpdateShaderInterface;

	void JDefaultMaterialSetting::SetStandard(const JUserPtr<JMaterial>& mat)
	{
		UpdateShaderInterface::OffUpdateShaderTrigger(mat);
		mat->SetShadow(true);
		mat->SetLight(true);
		UpdateShaderInterface::OnUpdateShaderTrigger(mat);
		UpdateShaderInterface::UpdateShader(mat);
	}
	void JDefaultMaterialSetting::SetSky(const JUserPtr<JMaterial>& mat, const JUserPtr<JTexture>& texture)
	{
		UpdateShaderInterface::OffUpdateShaderTrigger(mat);
		mat->SetSkyMaterial(true);
		//mat->SetNonCulling(true);
		//mat->SetDepthCompareFunc(J_SHADER_DEPTH_COMPARISON_FUNC::LESS_EQUAL);
		mat->SetAlbedoMap(texture);
		UpdateShaderInterface::OnUpdateShaderTrigger(mat);
		UpdateShaderInterface::UpdateShader(mat);
	}
	void JDefaultMaterialSetting::SetDebug(const JUserPtr<JMaterial>& mat, const bool isLine, const JVector4<float> color)
	{
		UpdateShaderInterface::OffUpdateShaderTrigger(mat);
		mat->SetDebugMaterial(true);
		if (isLine)
			mat->SetPrimitiveType(J_SHADER_PRIMITIVE_TYPE::LINE);
		mat->SetAlbedoColor(color); 
		mat->SetNonCulling(true);
		UpdateShaderInterface::OnUpdateShaderTrigger(mat);
		UpdateShaderInterface::UpdateShader(mat);
	}
	void JDefaultMaterialSetting::SetAlbedoMapOnly(const JUserPtr<JMaterial>& mat, const JUserPtr<JTexture>& texture)
	{
		UpdateShaderInterface::OffUpdateShaderTrigger(mat);
		mat->SetAlbedoMap(texture);
		mat->SetAlbedoMapOnly(true);
		UpdateShaderInterface::OnUpdateShaderTrigger(mat);
		UpdateShaderInterface::UpdateShader(mat);
	}
}