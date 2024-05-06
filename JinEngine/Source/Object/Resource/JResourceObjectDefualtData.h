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
#include"JResourceObjectType.h"
#include"JResourceObject.h"
#include"Material/JDefaultMaterialType.h"
#include"Mesh/JDefaultShapeType.h"
#include"Shader/JDefaultShaderType.h" 
#include"Texture/JDefaulTextureType.h"  
#include"JResourceObjectUserInterface.h"
//#include"../../Core/Identity/JIdentifier.h"
#include"../Directory/JFile.h" 
#include"../../Core/JCoreEssential.h"
#include"../../Core/Guid/JGuidCreator.h" 

namespace JinEngine
{ 
	class JTexture;
	class JStaticMeshGeometry;

	class JResourceObjectDefualtData : public JResourceObjectUserInterface
	{
	public:
		//uint editorTextureCount = 0; 
		// basic , standard, sky, skeleton
		// standard는 나중에 수정필요 런타임시 생성되므로
		// 나머지는 컴파일시 생성되고 고정값이므로 유지 
		static const std::vector<J_DEFAULT_TEXTURE> selectorTextureType;
		static const std::vector<J_DEFAULT_TEXTURE> projectTextureType;		 
		const size_t guid = Core::MakeGuid();
	private:
		//isUsed가 설정된 default resource만 보관
		//그 외에 default는 이미 생성되있으면 그 객체를 생성되 있지않으면 file을 통해서 load후 전달
		std::unordered_map<size_t, JUserPtr<JResourceObject>> defaultResourceMap;
	private:
		std::unordered_map<J_DEFAULT_TEXTURE, size_t> defaultTextureGuidMap;
		std::unordered_map<J_DEFAULT_GRAPHIC_SHADER, size_t> defaultGraphicShaderGuidMap;
		std::unordered_map<J_DEFAULT_COMPUTE_SHADER, size_t> defaultComputeShaderGuidMap;
		std::unordered_map<J_DEFAULT_MATERIAL, size_t> defaultMaterialGuidMap;
		std::unordered_map<J_DEFAULT_SHAPE, size_t> defaultMeshGuidMap;
	public:
		JResourceObjectDefualtData();
		~JResourceObjectDefualtData();
	public:
		void Initialize();
		void Clear(); 
	public:
		template<typename CastT, typename EnumName>
		JUserPtr<CastT> GetDefaultResource(const EnumName key)
		{
			if constexpr (std::is_same_v<EnumName, J_DEFAULT_TEXTURE>)
				return DoGetDefaultResource<CastT>(key, defaultTextureGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_GRAPHIC_SHADER>)
				return DoGetDefaultResource<CastT>(key, defaultGraphicShaderGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_COMPUTE_SHADER>)
				return DoGetDefaultResource<CastT>(key, defaultComputeShaderGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_MATERIAL>)
				return DoGetDefaultResource<CastT>(key, defaultMaterialGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_SHAPE>)
				return DoGetDefaultResource<CastT>(key, defaultMeshGuidMap);
			else
				return JUserPtr<CastT>{};
		}
		template<typename EnumName>
		size_t GetDefaultResourceGuid(const EnumName key, _Out_ bool& isSuccess)
		{
			if constexpr (std::is_same_v<EnumName, J_DEFAULT_TEXTURE>)
				return DoGetDefaultResourceGuid(key, defaultTextureGuidMap, isSuccess);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_GRAPHIC_SHADER>)
				return DoGetDefaultResourceGuid(key, defaultGraphicShaderGuidMap, isSuccess);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_COMPUTE_SHADER>)
				return DoGetDefaultResourceGuid(key, defaultComputeShaderGuidMap, isSuccess);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_MATERIAL>)
				return DoGetDefaultResourceGuid(key, defaultMaterialGuidMap, isSuccess);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_SHAPE>)
				return DoGetDefaultResourceGuid(key, defaultMeshGuidMap, isSuccess);
			else
			{
				isSuccess = false;
				return 0;
			}
		}
		template<typename EnumName>
		void RegisterDefaultResource(const EnumName key, const JUserPtr<JFile>& file, const bool isUsed)
		{
			if constexpr (std::is_same_v<EnumName, J_DEFAULT_TEXTURE>)
				DoRegisterDefaultResource(key, file, isUsed, defaultTextureGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_GRAPHIC_SHADER>)
				DoRegisterDefaultResource(key, file, isUsed, defaultGraphicShaderGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_COMPUTE_SHADER>)
				DoRegisterDefaultResource(key, file, isUsed, defaultComputeShaderGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_MATERIAL>)			
				DoRegisterDefaultResource(key, file, isUsed, defaultMaterialGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_SHAPE>)
				DoRegisterDefaultResource(key, file, isUsed, defaultMeshGuidMap);
		}
	private:
		template<typename CastT,  typename EnumName, typename GuidMap>
		JUserPtr<CastT> DoGetDefaultResource(const EnumName key, GuidMap& map)
		{
			auto guidData = map.find(key);
			if (guidData != map.end())
			{
				auto resourceData = defaultResourceMap.find(guidData->second);
				if (resourceData != defaultResourceMap.end())
				{
					JUserPtr<JResourceObject> resource = resourceData->second;
					return JUserPtr<CastT>::ConvertChild(std::move(resource));
				}
				else
				{
					auto file = JDirectory::SearchFile(guidData->second);
					return file != nullptr ? JUserPtr<CastT>::ConvertChild(file->TryGetResourceUser()) : JUserPtr<CastT>{};
				}
			}
			else
				return JUserPtr<CastT>{};
		}
		template<typename EnumName, typename GuidMap>
		size_t DoGetDefaultResourceGuid(const EnumName key, GuidMap& map, _Out_ bool& isSuccess)
		{
			auto guidData = map.find(key);
			if (guidData != map.end())
			{
				isSuccess = true;
				return guidData->second;
			}
			else
			{
				isSuccess = false;
				return 0;
			}
		}
		template<typename EnumName, typename GuidMap>
		void DoRegisterDefaultResource(const EnumName key,
			const JUserPtr<JFile>& file,
			const bool isUsed,
			GuidMap& map)
		{
			map.emplace(key, file->GetResourceGuid());
			//isUsed = true이며 resource가 load되지 않았을경우
			//resource을 file에서 불러온다.
			if (isUsed)
			{
				JUserPtr<JResourceObject> user = file->TryGetResourceUser();
				defaultResourceMap.emplace(user->GetGuid(), user);
				CallOnResourceReference(user.Get());
			}
		}
	private:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj, JResourceEventDesc* desc)final;
	};
}