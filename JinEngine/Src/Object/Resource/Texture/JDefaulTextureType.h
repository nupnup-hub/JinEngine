#pragma once
#include<string>
#include"../../JObjectFlag.h"

namespace JinEngine
{
	//has sequence dependency
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
			case JinEngine::J_DEFAULT_TEXTURE::POSITION_ARROW:
				return true;
			case JinEngine::J_DEFAULT_TEXTURE::ROTATION_ARROW:
				return true;
			case JinEngine::J_DEFAULT_TEXTURE::SCALE_ARROW:
				return true;
			case JinEngine::J_DEFAULT_TEXTURE::VIEW_FRUSTUM_ICON:
				return true;
			case JinEngine::J_DEFAULT_TEXTURE::COORD_GRID_ICON:
				return true;
			case JinEngine::J_DEFAULT_TEXTURE::OPTION_SETTING:
				return true;
			case JinEngine::J_DEFAULT_TEXTURE::PLAY_SCENE_TIME:
				return true;
			case JinEngine::J_DEFAULT_TEXTURE::PAUSE_SCENE_TIME:
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
			case JinEngine::J_DEFAULT_TEXTURE::PROJECT_SELECTOR_BACKGROUND:
				return L"Shiro_Background.jpg";
			default:
				break;
			}
		}
		static J_OBJECT_FLAG GetFlag(const J_DEFAULT_TEXTURE& texture)
		{
			switch (texture)
			{
			case JinEngine::J_DEFAULT_TEXTURE::DEFAULT_SKY:
				return (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE);
			case JinEngine::J_DEFAULT_TEXTURE::BASIC_MATERIAL_ALBEDO_MAP:
				return (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE);
			case JinEngine::J_DEFAULT_TEXTURE::BASIC_MATERIAL_NORMAL_MAP:
				return (J_OBJECT_FLAG)(OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE);
			default:
				return (J_OBJECT_FLAG)(OBJECT_FLAG_HIDDEN | OBJECT_FLAG_AUTO_GENERATED | OBJECT_FLAG_UNEDITABLE | OBJECT_FLAG_UNDESTROYABLE);
			}
		}
	};
	/*	const std::vector<JResourceData::JDefaultTextureInfo> JResourceData::selectorTextureType
	{
		{L"Reserved texture by Imgui", J_DEFAULT_TEXTURE::IMGUI_RESERVED},
		{L"Missing.jpg", J_DEFAULT_TEXTURE::MISSING},
		{L"None.png", J_DEFAULT_TEXTURE::NONE},
		{L"BlueSearchIcon.png", J_DEFAULT_TEXTURE::SEARCH_FOLDER_ICON},
		{L"Shiro_Background.jpg", J_DEFAULT_TEXTURE::PROJECT_SELECTOR_BACKGROUND},
	};
	const std::vector<JResourceData::JDefaultTextureInfo> JResourceData::projectTextureType
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