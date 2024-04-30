#pragma once
#include"../../../Object/Resource/Scene/Preview/JPreviewEnum.h" 
#include"../../../Object/JObjectType.h"
#include"../../../Core/JCoreEssential.h"
#include"../../../Core/Pointer/JOwnerPtr.h"
#include"../../../Core/Reflection/JTypeInfo.h"
#include<memory>	

namespace JinEngine
{
	class JPreviewSceneGroup;
	class JPreviewScene;
	class JObject;
	class JDirectory;

	namespace Editor
	{
		class JEditorPreviewImpl;
		class JEditorPageCounter;
		class JEditorPreviewInterface
		{
		private:
			std::unique_ptr<JEditorPreviewImpl> impl;
		protected:
			uint GetPreviewSceneCount()const noexcept;
			JPreviewScene* GetPreviewScene(const uint index)noexcept;
		protected:
			void SetPreviewGroupCapacity(const uint capacity)noexcept;
		protected:
			JPreviewScene* CreatePreviewScene(JUserPtr<JObject> jObj,
				const J_PREVIEW_DIMENSION previewDimension = J_PREVIEW_DIMENSION::TWO_DIMENTIONAL,
				const J_PREVIEW_FLAG previewFlag = J_PREVIEW_FLAG::NONE)noexcept;
			bool DestroyPreviewScene(JPreviewScene* prevewScene)noexcept;
			bool DestroyPreviewScene(JUserPtr<JObject> jObj)noexcept;
			void DestroyInvalidPreviewScene()noexcept;
			bool PopPreviewScene()noexcept;
			void ClearPreviewGroup()noexcept; 
		protected:
			void AlignByName(const bool isAscending = true)noexcept;
			void AlignByType(const J_OBJECT_TYPE type)noexcept;
		protected:
			std::vector<JPreviewScene*> TryCreateDirectoryPreview(const JUserPtr<JDirectory>& dir, 
				JEditorPageCounter* pageCounter, 
				Core::JTypeInfo* targetType = nullptr,
				const bool canCreatePreview = true);
		protected:
			JEditorPreviewInterface();
			virtual ~JEditorPreviewInterface();
		};
	}
}