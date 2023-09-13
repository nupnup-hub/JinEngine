#pragma once
#include"../../../Object/Resource/Scene/Preview/JPreviewEnum.h" 
#include"../../../Core/JCoreEssential.h"
#include"../../../Core/Pointer/JOwnerPtr.h"
#include<memory>	

namespace JinEngine
{
	class JPreviewSceneGroup;
	class JPreviewScene;
	class JObject;
	namespace Editor
	{
		class JEditorPreviewImpl;
		class JEditorPreviewInterface
		{
		private:
			std::unique_ptr<JEditorPreviewImpl> impl;
		protected:
			JPreviewScene* CreatePreviewScene(JUserPtr<JObject> jObj,
				const J_PREVIEW_DIMENSION previewDimension = J_PREVIEW_DIMENSION::TWO_DIMENTIONAL,
				const J_PREVIEW_FLAG previewFlag = J_PREVIEW_FLAG::NONE)noexcept;
			bool DestroyPreviewScene(JPreviewScene* prevewScene)noexcept;
			bool DestroyPreviewScene(JUserPtr<JObject> jObj)noexcept;
			void DestroyInvalidPreviewScene()noexcept;
			void ClearPreviewGroup()noexcept; 
		protected:
			uint GetPreviewSceneCount()const noexcept;
			JPreviewScene* GetPreviewScene(const uint index)noexcept;
			void SetPreviewGroupCapacity(const uint capacity)noexcept;
		protected:
			JEditorPreviewInterface();
			virtual ~JEditorPreviewInterface();
		};
	}
}