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


#include"JResourceObjectDefualtData.h"
#include"JResourceManager.h"
#include"Texture/JTexture.h" 
#include"Mesh/JStaticMeshGeometry.h"  

namespace JinEngine
{
	const std::vector<J_DEFAULT_TEXTURE> JResourceObjectDefualtData::selectorTextureType
	{
		J_DEFAULT_TEXTURE::MISSING,
		J_DEFAULT_TEXTURE::NONE,
		J_DEFAULT_TEXTURE::SEARCH_FOLDER_ICON,
		J_DEFAULT_TEXTURE::OPTION_SETTING,
		J_DEFAULT_TEXTURE::PROJECT_SELECTOR_BACKGROUND,
	};

	const std::vector<J_DEFAULT_TEXTURE> JResourceObjectDefualtData::projectTextureType
	{ 
		J_DEFAULT_TEXTURE::MISSING,
		J_DEFAULT_TEXTURE::NONE,
		J_DEFAULT_TEXTURE::DEFAULT_SKY,
		J_DEFAULT_TEXTURE::BASIC_MATERIAL_ALBEDO_MAP,
		J_DEFAULT_TEXTURE::BASIC_MATERIAL_NORMAL_MAP,
		J_DEFAULT_TEXTURE::BLUE_NOISE,
		J_DEFAULT_TEXTURE::DIRECTORY,
		J_DEFAULT_TEXTURE::TEXT,
		J_DEFAULT_TEXTURE::SCRIPT,
		J_DEFAULT_TEXTURE::SHADER,
		J_DEFAULT_TEXTURE::SKELETON,
		J_DEFAULT_TEXTURE::ANIMATION_CLIP,
		J_DEFAULT_TEXTURE::SCENE,
		J_DEFAULT_TEXTURE::ANIMATION_CONTROLLER,
		J_DEFAULT_TEXTURE::POSITION_ARROW,
		J_DEFAULT_TEXTURE::ROTATION_ARROW,
		J_DEFAULT_TEXTURE::SCALE_ARROW,
		J_DEFAULT_TEXTURE::VIEW_FRUSTUM_ICON,
		J_DEFAULT_TEXTURE::COORD_GRID_ICON,
		J_DEFAULT_TEXTURE::PLAY_SCENE_TIME, 
		J_DEFAULT_TEXTURE::PAUSE_SCENE_TIME,
		J_DEFAULT_TEXTURE::DIRECTIONAL_LIGHT,
		J_DEFAULT_TEXTURE::POINT_LIGHT,
		J_DEFAULT_TEXTURE::SPOT_LIGHT,
		J_DEFAULT_TEXTURE::RECT_LIGHT,
		J_DEFAULT_TEXTURE::CAMERA,
		J_DEFAULT_TEXTURE::LTC_MAT,
		J_DEFAULT_TEXTURE::LTC_AMP,
		J_DEFAULT_TEXTURE::PROJECT_SELECTOR_BACKGROUND		//Test
	}; 
	//const uint JResourceObjectDefualtData::editorTextureCapacity = (uint)textureNames.size(); 
	JResourceObjectDefualtData::JResourceObjectDefualtData()
	{}
	JResourceObjectDefualtData::~JResourceObjectDefualtData()
	{}
	void JResourceObjectDefualtData::Initialize()
	{
		AddEventListener(*JResourceObject::EvInterface(), guid, J_RESOURCE_EVENT_TYPE::ERASE_RESOURCE);
	}
	void JResourceObjectDefualtData::Clear()
	{
		RemoveListener(*JResourceObject::EvInterface(), guid);
		defaultMeshGuidMap.clear();
		defaultMaterialGuidMap.clear();
		defaultGraphicShaderGuidMap.clear();
		defaultComputeShaderGuidMap.clear();
		defaultTextureGuidMap.clear();
		defaultResourceMap.clear();
	}
	void JResourceObjectDefualtData::OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj, JResourceEventDesc* desc)
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