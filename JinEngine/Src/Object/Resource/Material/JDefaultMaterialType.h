#pragma once
#include"../Shader/JDefaultShaderType.h"
#include<string>

namespace JinEngine
{ 
	//has sequence dependency
	enum class J_DEFAULT_MATERIAL
	{
		DEFAULT_STANDARD,
		DEFAULT_SKY,
		DEFAULT_SHADOW_MAP,
		DEBUG_LINE_RED,
		DEBUG_LINE_GREEN,
		DEBUG_LINE_BLUE,
		DEBUG_LINE_YELLOW, 
		DEFAULT_BOUNDING_OBJECT_DEPTH_TEST,
		COUNTER
	};

	struct JDefaultMateiral
	{
	public: 
		static constexpr int debugTypeSt = (int)J_DEFAULT_MATERIAL::DEBUG_LINE_RED;
		static J_DEFAULT_GRAPHIC_SHADER FindMatchDefaultShaderType(const J_DEFAULT_MATERIAL materialType)
		{
			switch (materialType)
			{
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_STANDARD:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_STANDARD_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_SKY:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_SKY_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_SHADOW_MAP:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_SHADOW_MAP_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_RED:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_LINE_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_LINE_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_BLUE:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_LINE_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_LINE_SHADER; 
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_BOUNDING_OBJECT_DEPTH_TEST:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_BOUNDING_OBJECT_DEPTH_TEST_SHADER;
			}
			return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_STANDARD_SHADER;
		}
		static bool IsDefaultUse(const J_DEFAULT_MATERIAL type)
		{
			switch (type)
			{
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_STANDARD:
				return false;
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_SKY:
				return false;
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_SHADOW_MAP:
				return true;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_RED:
				return true;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN:
				return true;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_BLUE:
				return true;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW:
				return true;
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_BOUNDING_OBJECT_DEPTH_TEST:
				return true;
			default:
				return false;
			}
		}
		static std::wstring ConvertToName(const J_DEFAULT_MATERIAL type)
		{
			switch (type)
			{
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_STANDARD:
				return L"Default Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_SKY:
				return L"Default Sky Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_SHADOW_MAP:
				return L"Default Shadow Map Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_RED:
				return L"Default Debug BoundingBox Red Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN:
				return L"Default Debug BoundingBox Green Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_BLUE:
				return L"Default Debug BoundingBox Blue Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW:
				return L"Default Debug BoundingBox Yellow Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_BOUNDING_OBJECT_DEPTH_TEST:
				return L"Default Bounding Object Depth Test Material";
			default:
				return L"Error";
			}
		}
	};

}