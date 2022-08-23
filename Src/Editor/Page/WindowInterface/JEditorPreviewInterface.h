#pragma once
#include"../../../Object/Resource/Scene/Preview/JPreviewEnum.h" 
#include"../../../Core/JDataType.h"
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
			JPreviewScene* CreatePreviewScene(JObject* jObj,
				const J_PREVIEW_DIMENSION previewDimension, 
				const J_PREVIEW_FLAG previewFlag = J_PREVIEW_FLAG::NONE)noexcept;
			bool DestroyPreviewScene(const size_t sceneGuid)noexcept;
			bool DestroyPreviewScene(JObject* jObj)noexcept;
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