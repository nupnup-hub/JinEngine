#include"JResourceData.h"
#include"JResourceManager.h"
#include"Texture/JTexture.h" 
#include"../../Core/Guid/GuidCreator.h"  

namespace JinEngine
{
	JResourceData::DefaultTextureInfo::DefaultTextureInfo(const std::wstring& name, const J_EDITOR_TEXTURE type)
		:name(name), type(type)
	{}

	const std::vector<JResourceData::DefaultTextureInfo> JResourceData::selectorTexInfo
	{
		{L"Reserved texture by Imgui", J_EDITOR_TEXTURE::IMGUI_RESERVED},
		{L"Missing.jpg", J_EDITOR_TEXTURE::MISSING},
		{L"None.png", J_EDITOR_TEXTURE::NONE},
		{L"BlueSearchIcon.png", J_EDITOR_TEXTURE::SEARCH_FOLDER_ICON},
		{L"Shiro_Background.jpg", J_EDITOR_TEXTURE::PROJECT_SELECTOR_BACKGROUND},
	};

	const std::vector<JResourceData::DefaultTextureInfo> JResourceData::projectTexInfo
	{
		{L"Reserved texture by Imgui", J_EDITOR_TEXTURE::IMGUI_RESERVED},
		{L"Missing.jpg", J_EDITOR_TEXTURE::MISSING},
		{L"None.png", J_EDITOR_TEXTURE::NONE},
		{L"DefaultSky.dds", J_EDITOR_TEXTURE::DEFAULT_SKY},
		{L"white_Albedo.jpg", J_EDITOR_TEXTURE::BASIC_MATERIAL_DIFFUSE_MAP},
		{L"white_Normal.png", J_EDITOR_TEXTURE::BASIC_MATERIAL_NORMAL_MAP},
		{L"Dirctory.png", J_EDITOR_TEXTURE::DIRECTORY},
		{L"Text.png", J_EDITOR_TEXTURE::TEXT},
		{L"Script.png", J_EDITOR_TEXTURE::SCRIPT},
		{L"Shader.png", J_EDITOR_TEXTURE::SHADER},
		{L"skeleton.png", J_EDITOR_TEXTURE::SKELETON},
		{L"clip.png", J_EDITOR_TEXTURE::ANIMATION_CLIP},
		{L"Scene.png", J_EDITOR_TEXTURE::SCENE},
		{L"AniController.png", J_EDITOR_TEXTURE::ANIMATION_CONTROLLER},
	};
	std::unordered_map<J_EDITOR_TEXTURE, size_t> JResourceData::defaultTextureMap;

	//const uint JResourceData::editorTextureCapacity = (uint)textureNames.size();
	const std::string JResourceData::basicTextureFolder = "BasicResource";
	const std::vector<J_DEFAULT_SHAPE> JResourceData::defaultMeshTypes
	{
		J_DEFAULT_SHAPE::DEFAULT_SHAPE_CUBE,
		J_DEFAULT_SHAPE::DEFAULT_SHAPE_GRID,
		J_DEFAULT_SHAPE::DEFAULT_SHAPE_SPHERE,
		J_DEFAULT_SHAPE::DEFAULT_SHAPE_CYILINDER,
		J_DEFAULT_SHAPE::DEFAULT_SHAPE_QUAD,
		J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_BOX,
		J_DEFAULT_SHAPE::DEFAULT_SHAPE_BOUNDING_FRUSTUM
	};
	const std::vector<J_DEFAULT_MATERIAL> JResourceData::defaultMaterialTypes
	{
		J_DEFAULT_MATERIAL::DEFAULT_STANDARD,
		J_DEFAULT_MATERIAL::DEFAULT_SKY,
		J_DEFAULT_MATERIAL::DEFAULT_SHADOW_MAP,
		J_DEFAULT_MATERIAL::DEBUG_LINE_RED,
		J_DEFAULT_MATERIAL::DEBUG_LINE_GREEN,
		J_DEFAULT_MATERIAL::DEBUG_LINE_BLUE,
		J_DEFAULT_MATERIAL::DEBUG_LINE_YELLOW,
	};
	JResourceData::JResourceData()
		:guid(Core::MakeGuid())
	{}
	JResourceData::~JResourceData()
	{}
	void JResourceData::Initialize()
	{
		AddEventListener(*JResourceManager::Instance().EvInterface(), guid, J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
	}
	void JResourceData::Clear()
	{
		RemoveListener(*JResourceManager::Instance().EvInterface(), guid);
		defaultMeshGuidMap.clear();
		defaultMaterialGuidMap.clear();
		defaultShaderGuidMap.clear();
		missingTerxture.Clear();
		defaultTextureMap.clear();
	}
	void JResourceData::SetMissing(JTexture* texture)
	{
		CallOffResourceReference(missingTerxture.Get());
		missingTerxture = Core::GetUserPtr(texture);
		CallOnResourceReference(missingTerxture.Get());
	}
	void JResourceData::SetNone(JTexture* texture)
	{
		CallOffResourceReference(noneTerxture.Get());
		noneTerxture = Core::GetUserPtr(texture);
		CallOnResourceReference(noneTerxture.Get());
	}
	std::wstring JResourceData::FindEditTextureName(const J_EDITOR_TEXTURE type)const noexcept
	{
		for (const auto& data : selectorTexInfo)
		{
			if (data.type == type)
				return data.name;
		}
		for (const auto& data : projectTexInfo)
		{
			if (data.type == type)
				return data.name;
		}
	}
	void JResourceData::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
	{
		if (iden == guid)
			return;

		if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
		{
			if (missingTerxture.IsValid() && missingTerxture->GetGuid() == jRobj->GetGuid())
				SetMissing(nullptr); 
			else if (noneTerxture.IsValid() && noneTerxture->GetGuid() == jRobj->GetGuid())
				SetMissing(nullptr);
		}
	}
}