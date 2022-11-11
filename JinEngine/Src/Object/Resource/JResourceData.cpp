#include"JResourceData.h"
#include"JResourceManager.h"
#include"Texture/JTexture.h" 
#include"Mesh/JStaticMeshGeometry.h"
#include"../../Core/Guid/GuidCreator.h"  

namespace JinEngine
{
	const std::vector<J_DEFAULT_TEXTURE> JResourceData::selectorTextureType
	{
		J_DEFAULT_TEXTURE::MISSING,
		J_DEFAULT_TEXTURE::NONE,
		J_DEFAULT_TEXTURE::SEARCH_FOLDER_ICON,
		J_DEFAULT_TEXTURE::PROJECT_SELECTOR_BACKGROUND,
	};

	const std::vector<J_DEFAULT_TEXTURE> JResourceData::projectTextureType
	{ 
		J_DEFAULT_TEXTURE::MISSING,
		J_DEFAULT_TEXTURE::NONE,
		J_DEFAULT_TEXTURE::DEFAULT_SKY,
		J_DEFAULT_TEXTURE::BASIC_MATERIAL_DIFFUSE_MAP,
		J_DEFAULT_TEXTURE::BASIC_MATERIAL_NORMAL_MAP,
		J_DEFAULT_TEXTURE::DIRECTORY,
		J_DEFAULT_TEXTURE::TEXT,
		J_DEFAULT_TEXTURE::SCRIPT,
		J_DEFAULT_TEXTURE::SHADER,
		J_DEFAULT_TEXTURE::SKELETON,
		J_DEFAULT_TEXTURE::ANIMATION_CLIP,
		J_DEFAULT_TEXTURE::SCENE,
		J_DEFAULT_TEXTURE::ANIMATION_CONTROLLER,
	}; 
	//const uint JResourceData::editorTextureCapacity = (uint)textureNames.size(); 
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
		defaultTextureGuidMap.clear();
		defaultResourceMap.clear();
	}
	void JResourceData::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)
	{
		if (iden == guid)
			return;

		if (eventType == J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE)
		{
			auto data = defaultResourceMap.find(jRobj->GetGuid());
			if (data != defaultResourceMap.end())
				CallOffResourceReference(data->second.Get());
		}
	}
}