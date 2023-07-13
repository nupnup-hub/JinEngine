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
		DEBUG_RED,
		DEBUG_GREEN,
		DEBUG_BLUE,
		DEBUG_YELLOW,
		DEBUG_LINE_RED,
		DEBUG_LINE_GREEN,
		DEBUG_LINE_BLUE,
		DEBUG_LINE_YELLOW,
		DEBUG_LINE_GRAY,
		DEBUG_LINE_BLACK,
		DEFAULT_BOUNDING_OBJECT_DEPTH_TEST,
		COUNTER
	};

	class JDefaultMateiral
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
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_RED:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_GREEN:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_BLUE:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_YELLOW:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_RED:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_LINE_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_LINE_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_BLUE:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_LINE_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_LINE_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_GRAY:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_LINE_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_BLACK:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_DEBUG_LINE_SHADER;
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_BOUNDING_OBJECT_DEPTH_TEST:
				return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_BOUNDING_OBJECT_DEPTH_TEST_SHADER;
			}
			return J_DEFAULT_GRAPHIC_SHADER::DEFAULT_STANDARD_SHADER;
		}
		static J_OBJECT_FLAG GetFlag(const J_DEFAULT_MATERIAL type)
		{
			switch (type)
			{
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_STANDARD:
				return (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE);
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_SKY:
				return (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE);
			default:
				return (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_HIDDEN);
			}
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
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_RED:
				return true;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_GREEN:
				return true;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_BLUE:
				return true;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_YELLOW:
				return true;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_RED:
				return true;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN:
				return true;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_BLUE:
				return true;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW:
				return true;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_GRAY:
				return true;
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_BLACK:
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
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_RED:
				return L"Default Debug Red Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_GREEN:
				return L"Default Debug Green Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_BLUE:
				return L"Default Debug Blue Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_YELLOW:
				return L"Default Debug Yellow Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_RED:
				return L"Default Debug Line Red Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN:
				return L"Default Debug Line Green Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_BLUE:
				return L"Default Debug Line Blue Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW:
				return L"Default Debug Line Yellow Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_GRAY:
				return L"Default Debug Line Gray Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEBUG_LINE_BLACK:
				return L"Default Debug Line Black Material";
			case JinEngine::J_DEFAULT_MATERIAL::DEFAULT_BOUNDING_OBJECT_DEPTH_TEST:
				return L"Default Bounding Object Depth Test Material";
			default:
				return L"Error";
			}
		}
	};

}