#pragma once
#include<string>
#include<vector>
#include<unordered_map> 
#include"JResourceObjectType.h"
#include"JResourceObject.h"
#include"Material/JDefaultMaterialType.h"
#include"Mesh/JDefaultShapeType.h"
#include"Shader/JDefaultShaderType.h"
#include"Shader/JShaderFunctionEnum.h"
#include"Texture/JDefaulTextureType.h"  
#include"JResourceObjectUserInterface.h"
//#include"../../Core/Identity/JIdentifier.h"
#include"../Directory/JFile.h" 
#include"../../Core/JDataType.h"
#include"../../Core/Guid/GuidCreator.h" 

namespace JinEngine
{ 
	class JTexture;
	class JStaticMeshGeometry;

	class JResourceObjectDefualtData : public JResourceObjectUserInterface
	{
	public:
		//uint editorTextureCount = 0; 
		// basic , standard, sky, skeleton
		// standard�� ���߿� �����ʿ� ��Ÿ�ӽ� �����ǹǷ�
		// �������� �����Ͻ� �����ǰ� �������̹Ƿ� ���� 
		static const std::vector<J_DEFAULT_TEXTURE> selectorTextureType;
		static const std::vector<J_DEFAULT_TEXTURE> projectTextureType;		 
		const size_t guid = Core::MakeGuid();
	private:
		//isUse�� ������ default resource�� ����
		//�� �ܿ� default�� �̹� ������������ �� ��ü�� ������ ���������� file�� ���ؼ� load�� ����
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
		void RegisterDefaultResource(const EnumName key, const JUserPtr<JFile>& file, const bool isUse)
		{
			if constexpr (std::is_same_v<EnumName, J_DEFAULT_TEXTURE>)
				DoRegisterDefaultResource(key, file, isUse, defaultTextureGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_GRAPHIC_SHADER>)
				DoRegisterDefaultResource(key, file, isUse, defaultGraphicShaderGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_COMPUTE_SHADER>)
				DoRegisterDefaultResource(key, file, isUse, defaultComputeShaderGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_MATERIAL>)			
				DoRegisterDefaultResource(key, file, isUse, defaultMaterialGuidMap);
			else if constexpr (std::is_same_v<EnumName, J_DEFAULT_SHAPE>)
				DoRegisterDefaultResource(key, file, isUse, defaultMeshGuidMap);
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
					return JUserPtr<CastT>::ConvertChild(JDirectory::SearchFile(guidData->second)->TryGetResourceUser());
			}
			else
				return JUserPtr<CastT>{};
		}
		template<typename EnumName, typename GuidMap>
		void DoRegisterDefaultResource(const EnumName key,
			const JUserPtr<JFile>& file,
			const bool isUse,
			GuidMap& map)
		{
			map.emplace(key, file->GetResourceGuid());
			if (isUse)
			{
				JUserPtr<JResourceObject> user = file->TryGetResourceUser();
				defaultResourceMap.emplace(user->GetGuid(), user);
				CallOnResourceReference(user.Get());
			}
		}
	private:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
	};
}