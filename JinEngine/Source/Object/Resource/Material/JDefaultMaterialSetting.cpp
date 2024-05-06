/****************************************************************************************
MIT License

Copyright (c) 2021 jinwoo jung

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/


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