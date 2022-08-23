#include"JEditorPreviewInterface.h"
#include"../../../Object/Resource/JResourceManager.h"
#include"../../../Object/Resource/Scene/Preview/JPreviewSceneGroup.h"

namespace JinEngine
{
	namespace Editor
	{
		class JEditorPreviewImpl
		{
		public:
			std::unique_ptr<JPreviewSceneGroup> previewGroup;
		public:
			JEditorPreviewImpl()
			{
				previewGroup = std::make_unique<JPreviewSceneGroup>();
			}
			~JEditorPreviewImpl()
			{
				previewGroup.reset();
				previewGroup = nullptr;
			}
		public:
			JPreviewScene* CreatePreviewScene(JObject* jObj,
				const J_PREVIEW_DIMENSION previewDimension,
				const J_PREVIEW_FLAG previewFlag)noexcept
			{
				return previewGroup->CreatePreviewScene(jObj, previewDimension, previewFlag);
			}
			bool DestroyPreviewScene(const size_t sceneGuid)noexcept
			{
				return previewGroup != nullptr ? previewGroup->DestroyPreviewScene(sceneGuid) : false;
			}
			bool DestroyPreviewScene(JObject* jObj)noexcept
			{
				return previewGroup != nullptr ? previewGroup->DestroyPreviewScene(jObj) : false;
			}
			void ClearPreviewGroup()noexcept
			{
				if (previewGroup != nullptr)
					previewGroup->Clear();
			}
			uint GetPreviewSceneCount()const noexcept
			{
				if (previewGroup != nullptr)
					return previewGroup->GetPreviewSceneCount();
			}
			JPreviewScene* GetPreviewScene(const uint index)noexcept
			{
				if (previewGroup != nullptr)
					return previewGroup->GetPreviewScene(index);
			}
			void SetPreviewGroupCapacity(const uint capacity)noexcept
			{
				if (previewGroup != nullptr)
					previewGroup->SetCapacity(capacity);
			}
		};

		JPreviewScene* JEditorPreviewInterface::CreatePreviewScene(JObject* jObj,
			const J_PREVIEW_DIMENSION previewDimension,
			const J_PREVIEW_FLAG previewFlag)noexcept
		{
			return impl->CreatePreviewScene(jObj, previewDimension, previewFlag);
		}
		bool JEditorPreviewInterface::DestroyPreviewScene(const size_t sceneGuid)noexcept
		{
			return impl->DestroyPreviewScene(sceneGuid);
		}
		bool JEditorPreviewInterface::DestroyPreviewScene(JObject* jObj)noexcept
		{
			return impl->DestroyPreviewScene(jObj);
		}
		void JEditorPreviewInterface::ClearPreviewGroup()noexcept
		{
			return impl->ClearPreviewGroup();
		}
		uint JEditorPreviewInterface::GetPreviewSceneCount()const noexcept
		{
			return impl->GetPreviewSceneCount();
		}
		JPreviewScene* JEditorPreviewInterface::GetPreviewScene(const uint index)noexcept
		{
			return impl->GetPreviewScene(index);
		}
		void JEditorPreviewInterface::SetPreviewGroupCapacity(const uint capacity)noexcept
		{
			impl->SetPreviewGroupCapacity(capacity);
		}
		JEditorPreviewInterface::JEditorPreviewInterface()
		{
			impl = std::make_unique<JEditorPreviewImpl>();
		}
		JEditorPreviewInterface::~JEditorPreviewInterface()
		{
			impl.reset();
			impl = nullptr;
		}
	}
}