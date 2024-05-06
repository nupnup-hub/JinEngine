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


#pragma once 
#include"../../JObjectFlag.h"
#include"../../../Graphic/Shader/JShaderType.h"

namespace JinEngine
{
	enum class J_DEFAULT_GRAPHIC_SHADER
	{
		DEFAULT_SHADER,			
		DEFAULT_LIGHT_SHADER,
		DEFAULT_ALBEDOMAP_SHADER,
		DEFAULT_STANDARD_SHADER,
		DEFAULT_SKY_SHADER, 
		DEFAULT_DEBUG_SHADER,
		DEFAULT_DEBUG_LINE_SHADER, 
		DEFAULT_PREVIEW_TEXTURE_SHADER,
		COUNTER,
	};

	enum class J_DEFAULT_COMPUTE_SHADER
	{ 
		COUNTER,
	};
	class JDefaultShader
	{
	public:
		static J_GRAPHIC_SHADER_FUNCTION GetShaderFunction(const J_DEFAULT_GRAPHIC_SHADER sType)
		{ 
			switch (sType)
			{
			case JinEngine::J_DEFAULT_GRAPHIC_SHADER::DEFAULT_SHADER:
				return SHADER_FUNCTION_NONE;
			case JinEngine::J_DEFAULT_GRAPHIC_SHADER::DEFAULT_LIGHT_SHADER:
				return SHADER_FUNCTION_LIGHT;
			case JinEngine::J_DEFAULT_GRAPHIC_SHADER::DEFAULT_ALBEDOMAP_SHADER:
				return SHADER_FUNCTION_ALBEDO_MAP;
			case JinEngine::J_DEFAULT_GRAPHIC_SHADER::DEFAULT_STANDARD_SHADER:
				return (J_GRAPHIC_SHADER_FUNCTION)(SHADER_FUNCTION_SHADOW | SHADER_FUNCTION_LIGHT);
			case JinEngine::J_DEFAULT_GRAPHIC_SHADER::DEFAULT_SKY_SHADER:
				return SHADER_FUNCTION_SKY; 
			case JinEngine::J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_SHADER:
				return SHADER_FUNCTION_DEBUG; 
			case JinEngine::J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_LINE_SHADER:
				return SHADER_FUNCTION_DEBUG; 
			case JinEngine::J_DEFAULT_GRAPHIC_SHADER::DEFAULT_PREVIEW_TEXTURE_SHADER:
				return(J_GRAPHIC_SHADER_FUNCTION)(SHADER_FUNCTION_ALBEDO_MAP_ONLY | SHADER_FUNCTION_ALBEDO_MAP);
			default:
				return SHADER_FUNCTION_NONE;
			}
		}
		static JGraphicShaderCondition GetShaderGraphicPso(const J_DEFAULT_GRAPHIC_SHADER sType)
		{ 
			switch (sType)
			{
			case JinEngine::J_DEFAULT_GRAPHIC_SHADER::DEFAULT_SKY_SHADER:
			{
				JGraphicShaderCondition pso;
				pso.cullModeCondition = J_SHADER_APPLIY_CONDITION::APPLY;
				pso.isCullModeNone = true;
				pso.depthCompareCondition = J_SHADER_APPLIY_CONDITION::APPLY;
				pso.depthCompareFunc = J_SHADER_DEPTH_COMPARISON_FUNC::LESS_EQUAL;
				return pso;
			}
			case JinEngine::J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_LINE_SHADER:
			{
				JGraphicShaderCondition pso;
				pso.primitiveCondition = J_SHADER_APPLIY_CONDITION::APPLY;
				pso.primitiveType = J_SHADER_PRIMITIVE_TYPE::LINE; 
				return pso;
			}
			default:
				return JGraphicShaderCondition{};
			}
		}
		static J_COMPUTE_SHADER_FUNCTION GetComputeShaderFunction(const J_DEFAULT_COMPUTE_SHADER cType)
		{
			return J_COMPUTE_SHADER_FUNCTION::NONE;
			//switch (cType)
			//{ 
			//default:
			//	return J_COMPUTE_SHADER_FUNCTION::NONE;
			//}
		}
		static J_OBJECT_FLAG GetObjectFlag(const J_DEFAULT_GRAPHIC_SHADER sType)
		{
			switch (sType)
			{
			case JinEngine::J_DEFAULT_GRAPHIC_SHADER::DEFAULT_STANDARD_SHADER:
				return (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE);
			case JinEngine::J_DEFAULT_GRAPHIC_SHADER::DEFAULT_SKY_SHADER:
				return (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE);
			default:
				return (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE);
			}
		}
		static J_OBJECT_FLAG GetObjectFlag(const J_DEFAULT_COMPUTE_SHADER cType)
		{
			return (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_HIDDEN | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE);
		}

		static bool IsDefaultUse(const J_DEFAULT_GRAPHIC_SHADER cType)
		{ 
			return true;
		 /*
			switch (cType)
			{
			default:
				return true;
			}
		 */
		}
		static bool IsDefaultUsed(const J_DEFAULT_COMPUTE_SHADER cType)
		{
			return false;
			//switch (cType)
			///{ 
			//default:
			//	return false;
			//}
		}
	};
}