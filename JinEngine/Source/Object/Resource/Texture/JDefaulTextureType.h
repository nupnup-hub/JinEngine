#pragma once 
#include"../../JObjectFlag.h"

namespace JinEngine
{
	//has sequence dependency
	//has to definition project per J_DEFAULT_TEXTURE in JResourceObjectDefualtData.h
	enum class J_DEFAULT_TEXTURE
	{
		NONE = 0, 
		DIRECTORY,
		TEXT,
		SCRIPT, 
		SHADER,
		SEARCH_FOLDER_ICON, 
		SCENE, 
		BASIC_MATERIAL_ALBEDO_MAP,
		BASIC_MATERIAL_NORMAL_MAP,
		DEFAULT_SKY,
		BLUE_NOISE,
		SKELETON,
		ANIMATION_CLIP,
		ANIMATION_CONTROLLER,
		MISSING, 
		POSITION_ARROW,
		ROTATION_ARROW,
		SCALE_ARROW,
		VIEW_FRUSTUM_ICON,
		COORD_GRID_ICON,
		OPTION_SETTING,
		PLAY_SCENE_TIME,
		PAUSE_SCENE_TIME,
		DIRECTIONAL_LIGHT,
		POINT_LIGHT,
		SPOT_LIGHT,
		RECT_LIGHT,
		CAMERA,
		LTC_MAT,	//32x32
		LTC_AMP,	//32x32
		PROJECT_SELECTOR_BACKGROUND
	};

	class JDefaultTexture
	{
	public:
		static bool IsDefaultUse(const J_DEFAULT_TEXTURE& texture)
		{
			switch (texture)
			{
			case JinEngine::J_DEFAULT_TEXTURE::NONE:
				return true;
			case JinEngine::J_DEFAULT_TEXTURE::MISSING:
				return true;
			case JinEngine::J_DEFAULT_TEXTURE::BLUE_NOISE:
				return true;
			case JinEngine::J_DEFAULT_TEXTURE::LTC_MAT:
				return true;
			case JinEngine::J_DEFAULT_TEXTURE::LTC_AMP:
				return true;
			default:
				return false;
			}
		}
		static std::wstring GetName(const J_DEFAULT_TEXTURE& texture)
		{			
			switch (texture)
			{
			case JinEngine::J_DEFAULT_TEXTURE::NONE:
				return L"None.png"; 
			case JinEngine::J_DEFAULT_TEXTURE::DIRECTORY:
				return L"Dirctory.png";
			case JinEngine::J_DEFAULT_TEXTURE::TEXT:
				return L"Text.png";
			case JinEngine::J_DEFAULT_TEXTURE::SCRIPT:
				return L"Script.png"; 
			case JinEngine::J_DEFAULT_TEXTURE::SHADER:
				return L"Shader.png";
			case JinEngine::J_DEFAULT_TEXTURE::SEARCH_FOLDER_ICON:
				return L"BlueSearchIcon.png"; 
			case JinEngine::J_DEFAULT_TEXTURE::SCENE:
				return L"Scene.png"; 
			case JinEngine::J_DEFAULT_TEXTURE::BASIC_MATERIAL_ALBEDO_MAP:
				return L"white_Albedo.jpg";
			case JinEngine::J_DEFAULT_TEXTURE::BASIC_MATERIAL_NORMAL_MAP:
				return L"white_Normal.png";
			case JinEngine::J_DEFAULT_TEXTURE::DEFAULT_SKY:
				return L"DefaultSky.dds";
			case JinEngine::J_DEFAULT_TEXTURE::BLUE_NOISE:
				return L"BlueNoise.png";
			case JinEngine::J_DEFAULT_TEXTURE::SKELETON:
				return L"skeleton.png";
			case JinEngine::J_DEFAULT_TEXTURE::ANIMATION_CLIP:
				return L"clip.png";
			case JinEngine::J_DEFAULT_TEXTURE::ANIMATION_CONTROLLER:
				return L"AniController.png";
			case JinEngine::J_DEFAULT_TEXTURE::MISSING:
				return L"Missing.jpg";
			case JinEngine::J_DEFAULT_TEXTURE::POSITION_ARROW:
				return L"PositionArrow.png";
			case JinEngine::J_DEFAULT_TEXTURE::ROTATION_ARROW:
				return L"RotationArrow.png";
			case JinEngine::J_DEFAULT_TEXTURE::SCALE_ARROW:
				return L"ScaleArrow.png";
			case JinEngine::J_DEFAULT_TEXTURE::VIEW_FRUSTUM_ICON:
				return L"Viewfrustum.png";
			case JinEngine::J_DEFAULT_TEXTURE::COORD_GRID_ICON:
				return L"CoordGrid.png";
			case JinEngine::J_DEFAULT_TEXTURE::OPTION_SETTING:
				return L"OptionSetting.png";
			case JinEngine::J_DEFAULT_TEXTURE::PLAY_SCENE_TIME:
				return L"PlaySceneTime.png";
			case JinEngine::J_DEFAULT_TEXTURE::PAUSE_SCENE_TIME:
				return L"PauseSceneTime.png";
			case JinEngine::J_DEFAULT_TEXTURE::DIRECTIONAL_LIGHT:
				return L"DirectionalLight.png";
			case JinEngine::J_DEFAULT_TEXTURE::POINT_LIGHT:
				return L"PointLight.png";
			case JinEngine::J_DEFAULT_TEXTURE::SPOT_LIGHT:
				return L"SpotLight.png";
			case JinEngine::J_DEFAULT_TEXTURE::RECT_LIGHT:
				return L"RectLight.png";
			case JinEngine::J_DEFAULT_TEXTURE::CAMERA:
				return L"Camera.png";
			case JinEngine::J_DEFAULT_TEXTURE::LTC_MAT:
				return L"LTC_Mat.dds";
			case JinEngine::J_DEFAULT_TEXTURE::LTC_AMP:
				return L"LTC_Amp.dds";
			case JinEngine::J_DEFAULT_TEXTURE::PROJECT_SELECTOR_BACKGROUND:
				return L"Shiro_Background.jpg";
			default:
				return L"Invalid";
			}
		}
		static J_OBJECT_FLAG GetFlag(const J_DEFAULT_TEXTURE& texture)
		{
			switch (texture)
			{
			case JinEngine::J_DEFAULT_TEXTURE::DEFAULT_SKY:
				return (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE);
			case JinEngine::J_DEFAULT_TEXTURE::BASIC_MATERIAL_ALBEDO_MAP:
				return (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE);
			case JinEngine::J_DEFAULT_TEXTURE::BASIC_MATERIAL_NORMAL_MAP:
				return (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE);
			default:
				return (J_OBJECT_FLAG)(OBJECT_FLAG_HIDDEN | OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE | OBJECT_FLAG_RESTRICT_CONTROL_IDENTIFICABLE);
			}
		}
	};
	/*	const std::vector<JResourceObjectDefualtData::JDefaultTextureInfo> JResourceObjectDefualtData::selectorTextureType
	{
		{L"Reserved texture by Imgui", J_DEFAULT_TEXTURE::IMGUI_RESERVED},
		{L"Missing.jpg", J_DEFAULT_TEXTURE::MISSING},
		{L"None.png", J_DEFAULT_TEXTURE::NONE},
		{L"BlueSearchIcon.png", J_DEFAULT_TEXTURE::SEARCH_FOLDER_ICON},
		{L"Shiro_Background.jpg", J_DEFAULT_TEXTURE::PROJECT_SELECTOR_BACKGROUND},
	};
	const std::vector<JResourceObjectDefualtData::JDefaultTextureInfo> JResourceObjectDefualtData::projectTextureType
	{
		{L"Reserved texture by Imgui", J_DEFAULT_TEXTURE::IMGUI_RESERVED},
		{L"Missing.jpg", J_DEFAULT_TEXTURE::MISSING},
		{L"None.png", J_DEFAULT_TEXTURE::NONE},
		{L"DefaultSky.dds", J_DEFAULT_TEXTURE::DEFAULT_SKY},
		{L"white_Albedo.jpg", J_DEFAULT_TEXTURE::BASIC_MATERIAL_ALBEDO_MAP},
		{L"white_Normal.png", J_DEFAULT_TEXTURE::BASIC_MATERIAL_NORMAL_MAP},
		{L"Dirctory.png", J_DEFAULT_TEXTURE::DIRECTORY},
		{L"Text.png", J_DEFAULT_TEXTURE::TEXT},
		{L"Script.png", J_DEFAULT_TEXTURE::SCRIPT},
		{L"Shader.png", J_DEFAULT_TEXTURE::SHADER},
		{L"skeleton.png", J_DEFAULT_TEXTURE::SKELETON},
		{L"clip.png", J_DEFAULT_TEXTURE::ANIMATION_CLIP},
		{L"Scene.png", J_DEFAULT_TEXTURE::SCENE},
		{L"AniController.png", J_DEFAULT_TEXTURE::ANIMATION_CONTROLLER},
	};
	*/
}