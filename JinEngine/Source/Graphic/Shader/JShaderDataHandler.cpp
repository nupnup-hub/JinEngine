#include"JShaderDataHandler.h"
#include"../../Object/Resource/Shader/JShader.h"
#include"../../Object/Resource/Shader/JShaderPrivate.h"

namespace JinEngine::Graphic
{ 
	void JGraphicShaderDataHandler::RecompileUserShader(JShader* shader)
	{
		JShaderPrivate::CompileInterface::RecompileGraphicShader(shader);
	}
	void JComputeShaderDataHandler::RecompileShader(JShader* shader)
	{
		JShaderPrivate::CompileInterface::RecompileComputeShader(shader);
	}
}