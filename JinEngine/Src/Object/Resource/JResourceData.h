#pragma once
#include<string>
#include<vector>
#include<unordered_map> 
#include"JResourceType.h"
#include"JResourceObject.h"
#include"Material/JDefaultMaterialType.h"
#include"Mesh/JDefaultShapeType.h"
#include"Shader/JDefaultShaderType.h"
#include"Shader/JShaderFunctionEnum.h"
#include"Texture/JDefaulTextureType.h"  
#include"JResourceUserInterface.h"
//#include"../../Core/Identity/JIdentifier.h"
#include"../../Core/JDataType.h"

namespace JinEngine
{
	class JTexture;
	class JStaticMeshGeometry;

	class JResourceData : public JResourceUserInterface
	{
	public:
		//uint editorTextureCount = 0; 
		// basic , standard, sky, skeleton
		// standard는 나중에 수정필요 런타임시 생성되므로
		// 나머지는 컴파일시 생성되고 고정값이므로 유지 
		static const std::vector<J_DEFAULT_TEXTURE> selectorTextureType;
		static const std::vector<J_DEFAULT_TEXTURE> projectTextureType;		 
		const size_t guid;
	private:
		std::unordered_map<size_t, Core::JUserPtr<JResourceObject>> defaultResourceMap;
		std::unordered_map<J_DEFAULT_TEXTURE, size_t> defaultTextureGuidMap;
		std::unordered_map<J_DEFAULT_GRAPHIC_SHADER, size_t> defaultGraphicShaderGuidMap;
		std::unordered_map<J_DEFAULT_COMPUTE_SHADER, size_t> defaultComputeShaderGuidMap;
		std::unordered_map<J_DEFAULT_MATERIAL, size_t> defaultMaterialGuidMap;
		std::unordered_map<J_DEFAULT_SHAPE, size_t> defaultMeshGuidMap;
	public:
		JResourceData();
		~JResourceData();
	public:
		void Initialize();
		void Clear();
	public:
		template<typename EnumName>
		JResourceObject* GetDefaultResource(const EnumName key)
		{
			if constexpr (std::is_same_v<EnumName, J_DEFAULT_TEXTURE>)
			{
				auto data = defaultTextureGuidMap.find(key);
				if (data != defaultTextureGuidMap.end())
					return defaultResourceMap.find(data->second)->second.Get();
				else
					return nullptr;
			}
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_GRAPHIC_SHADER>)
			{
				auto data = defaultGraphicShaderGuidMap.find(key);
				if (data != defaultGraphicShaderGuidMap.end())
					return defaultResourceMap.find(data->second)->second.Get();
				else
					return nullptr;
			}
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_COMPUTE_SHADER>)
			{
				auto data = defaultComputeShaderGuidMap.find(key);
				if (data != defaultComputeShaderGuidMap.end())
					return defaultResourceMap.find(data->second)->second.Get();
				else
					return nullptr;
			}
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_MATERIAL>)
			{
				auto data = defaultMaterialGuidMap.find(key);
				if (data != defaultMaterialGuidMap.end())
					return defaultResourceMap.find(data->second)->second.Get();
				else
					return nullptr;
			}
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_SHAPE>)
			{
				auto data = defaultMeshGuidMap.find(key);
				if (data != defaultMeshGuidMap.end())
					return defaultResourceMap.find(data->second)->second.Get();
				else
					return nullptr;
			}
			else
				return nullptr;
		}
		template<typename EnumName>
		void RegisterDefaultResource(const EnumName key, const Core::JUserPtr<JResourceObject> userObj, const bool isUse)
		{
			if constexpr (std::is_same_v<EnumName, J_DEFAULT_TEXTURE>)
				DoRegisterDefaultResource(key, userObj, isUse, defaultTextureGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_GRAPHIC_SHADER>)
				DoRegisterDefaultResource(key, userObj, isUse, defaultGraphicShaderGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_COMPUTE_SHADER>)
				DoRegisterDefaultResource(key, userObj, isUse, defaultComputeShaderGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_MATERIAL>)			
				DoRegisterDefaultResource(key, userObj, isUse, defaultMaterialGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_SHAPE>)
				DoRegisterDefaultResource(key, userObj, isUse, defaultMeshGuidMap);
		}
	private:
		template<typename EnumName, typename GuidMap>
		void DoRegisterDefaultResource(const EnumName key,
			const Core::JUserPtr<JResourceObject> userObj,
			const bool isUse,
			GuidMap& map)
		{
			map.emplace(key, userObj->GetGuid());
			defaultResourceMap.emplace(userObj->GetGuid(), userObj);
			if (isUse)
				CallOnResourceReference(userObj.Get());
		}
	private:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
	};
}