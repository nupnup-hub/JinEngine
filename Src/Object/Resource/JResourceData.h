#pragma once
#include<string>
#include<vector>
#include<unordered_map> 
#include"JResourceType.h"
#include"Material/JDefaultMaterialType.h"
#include"Mesh/JDefaultShapeType.h"
#include"Shader/JDefaultShaderType.h"
#include"Shader/JShaderFunctionEnum.h"
#include"Texture/JEditorTextureEnum.h" 
#include"JResourceUserInterface.h"
#include"../../Core/JDataType.h"

namespace JinEngine
{
	class JTexture;
	class JResourceData : public JResourceUserInterface
	{
	public:
		struct DefaultTextureInfo
		{
		public:
			const std::wstring name;
			const J_EDITOR_TEXTURE type;
		public:
			DefaultTextureInfo(const std::wstring& name, const J_EDITOR_TEXTURE type);
		};
	public:
		//uint editorTextureCount = 0; 
		// basic , standard, sky, skeleton
		// standard는 나중에 수정필요 런타임시 생성되므로
		// 나머지는 컴파일시 생성되고 고정값이므로 유지 
		static const std::vector<DefaultTextureInfo> selectorTexInfo;
		static const std::vector<DefaultTextureInfo> projectTexInfo;
		static std::unordered_map<J_EDITOR_TEXTURE, size_t> defaultTextureMap;

		//static const uint editorTextureCapacity;
		static const std::string basicTextureFolder;

		static const std::unordered_map<std::string, J_RESOURCE_TYPE>resourceType;
		static const std::unordered_map <J_RESOURCE_TYPE, std::string> newResourceDefaultName;

		static const std::vector<J_DEFAULT_SHAPE> defaultMeshTypes;
		static const std::vector<J_DEFAULT_MATERIAL> defaultMaterialTypes;
		 
		std::unordered_map<J_DEFAULT_SHAPE, size_t> defaultMeshGuidMap;
		std::unordered_map<J_DEFAULT_MATERIAL, size_t> defaultMaterialGuidMap;
		std::unordered_map<J_DEFAULT_SHADER, size_t> defaultShaderGuidMap;
		Core::JUserPtr<JTexture> missingTerxture;
		Core::JUserPtr<JTexture> noneTerxture;
		const size_t guid;

		// Init Resource Capacity
		static constexpr  uint initMeshCapacity = 250; 
		static constexpr  uint initMaterialCapacity = 150;
		static constexpr  uint initTextureCapacity = 150;
		static constexpr  uint initGameObjectCapacity = 10000; 
		static constexpr  uint initSceneCapacity = 8;
		static constexpr  uint initSkeletonCapacity = 25;
		static constexpr  uint initAnimationClipCapacity = 25;
		static constexpr  uint initAnimationControllerCapacity = 8;
		static constexpr  uint initShaderCapacity = 25;
		static constexpr  uint initDirectoryCapacity = 20;
		//JScene Viewer 2 + JScene Editor 1
		static constexpr  uint initSceneCameraCapacity = 16;
		static constexpr  uint initPreviewSceneCapacity = 150;
	public:
		JResourceData();
		~JResourceData();
	public:
		void Initialize();
		void Clear();
		void SetMissing(JTexture* texture);
		void SetNone(JTexture* texture);
	public:
		std::wstring FindEditTextureName(const J_EDITOR_TEXTURE type)const noexcept;
	private:
		void OnEvent(const size_t& iden, const J_RESOURCE_EVENT_TYPE& eventType, JResourceObject* jRobj)final;
	};
}