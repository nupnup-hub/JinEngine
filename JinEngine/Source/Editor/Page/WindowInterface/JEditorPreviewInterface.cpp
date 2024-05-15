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


#include"JEditorPreviewInterface.h"
#include"../../EditTool/JEditorPageCounter.h"
#include"../../../Object/Resource/JResourceManager.h"
#include"../../../Object/Resource/Scene/Preview/JPreviewSceneGroup.h"
#include"../../../Object/Resource/Scene/Preview/JPreviewScene.h"
#include"../../../Object/Resource/JResourceObject.h"
#include"../../../Object/Directory/JDirectory.h"
#include"../../../Object/Directory/JFile.h" 
 
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
				previewGroup = nullptr;
			}
		public:
			uint GetPreviewSceneCount()const noexcept
			{
				if (previewGroup != nullptr)
					return previewGroup->GetPreviewSceneCount();
				else
					return 0;
			}
			JPreviewScene* GetPreviewScene(const uint index)noexcept
			{
				if (previewGroup != nullptr)
					return previewGroup->GetPreviewScene(index);
				else
					return nullptr;
			}
		public:
			void SetPreviewGroupCapacity(const uint capacity)noexcept
			{
				if (previewGroup != nullptr)
					previewGroup->SetCapacity(capacity);
			}
		public:
			JPreviewScene* CreatePreviewScene(JUserPtr<JObject> jObj,
				const J_PREVIEW_DIMENSION previewDimension,
				const J_PREVIEW_FLAG previewFlag)noexcept
			{ 
				return previewGroup->CreatePreviewScene(jObj, previewDimension, previewFlag);
			}
			bool DestroyPreviewScene(JPreviewScene* previewScene)noexcept
			{ 
				return previewGroup != nullptr ? previewGroup->DestroyPreviewScene(previewScene) : false;
			}
			bool DestroyPreviewScene(JUserPtr<JObject> jObj)noexcept
			{ 
				return previewGroup != nullptr ? previewGroup->DestroyPreviewScene(jObj) : false;
			}
			void DestroyInvalidPreviewScene()noexcept
			{
				if (previewGroup != nullptr)
					previewGroup->DestroyInvalidPreviewScene(); 
			}
			bool PopPreviewScene()noexcept 
			{
				return previewGroup != nullptr ? previewGroup->PopPreviewScene() : false;
			}
			void ClearPreviewGroup()noexcept
			{ 
				if (previewGroup != nullptr)
					previewGroup->Clear();
			}
		public:
			void AlignByName(const bool isAscending)noexcept
			{
				if (previewGroup != nullptr)
					previewGroup->AlignByName(isAscending);
			}
			void AlignByType(const J_OBJECT_TYPE type)noexcept
			{
				if (previewGroup != nullptr)
					previewGroup->AlignByType(type); 
			}
		};

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
		JPreviewScene* JEditorPreviewInterface::CreatePreviewScene(JUserPtr<JObject> jObj,
			const J_PREVIEW_DIMENSION previewDimension,
			const J_PREVIEW_FLAG previewFlag)noexcept
		{
			if (!jObj.IsValid())
				return nullptr;

			return impl->CreatePreviewScene(jObj, previewDimension, previewFlag);
		}
		bool JEditorPreviewInterface::DestroyPreviewScene(JPreviewScene* previewScene)noexcept
		{
			if (previewScene == nullptr)
				return false;
			return impl->DestroyPreviewScene(previewScene);
		}
		bool JEditorPreviewInterface::DestroyPreviewScene(JUserPtr<JObject> jObj)noexcept
		{
			return impl->DestroyPreviewScene(jObj);
		}
		void JEditorPreviewInterface::DestroyInvalidPreviewScene()noexcept
		{
			impl->DestroyInvalidPreviewScene();
		}
		bool JEditorPreviewInterface::PopPreviewScene()noexcept
		{
			return impl->PopPreviewScene();
		}
		void JEditorPreviewInterface::ClearPreviewGroup()noexcept
		{
			return impl->ClearPreviewGroup();
		}
		void JEditorPreviewInterface::AlignByName(const bool isAscending)noexcept
		{
			impl->AlignByName(isAscending);
		}
		void JEditorPreviewInterface::AlignByType(const J_OBJECT_TYPE type)noexcept
		{
			impl->AlignByType(type);
		}
		std::vector<JPreviewScene*> JEditorPreviewInterface::TryCreateDirectoryPreview(const JUserPtr<JDirectory>& dir,
			JEditorPageCounter* pageCounter, 
			Core::JTypeInfo* targetType,
			const bool canCreatePreview)
		{ 
			std::vector<JPreviewScene*> newPreviewScene; 
			auto fileVec = dir->GetDirectoryFileVec(false);
			auto childDirectoryVec = dir->GetChildDirctoryVec();
			const uint existPreviewCount = GetPreviewSceneCount();

			JDirectory::AlignByName(fileVec);
			Core::JIdentifier::AlignByName(childDirectoryVec);

			pageCounter->BeginCounting();
			for (auto& file : fileVec)
			{
				if (Core::HasSQValueEnum(file->GetObjectFlag(), OBJECT_FLAG_HIDDEN) || (targetType != nullptr && !file->GetResourceTypeInfo().IsChildOf(*targetType)))
					continue;

				const bool isValidIndex = pageCounter->IsValidIndex();
				pageCounter->Count();
				if (existPreviewCount > 0)
				{
					bool hasOverlap = false;
					for (uint i = 0; i < existPreviewCount; ++i)
					{
						if (GetPreviewScene(i)->GetJObject()->GetGuid() == file->GetResourceGuid())
						{
							hasOverlap = true;
							break;
						}
					}
					if (hasOverlap)
						continue;
				}
				if (canCreatePreview && isValidIndex)
					newPreviewScene.push_back(CreatePreviewScene(file->TryGetResourceUser()));
			}
			for (auto& dir : childDirectoryVec)
			{
				if (Core::HasSQValueEnum(dir->GetFlag(), OBJECT_FLAG_HIDDEN))
					continue;

				const bool isValidIndex = pageCounter->IsValidIndex();
				pageCounter->Count();
				if (existPreviewCount > 0)
				{
					bool hasOverlap = false;
					for (uint i = 0; i < existPreviewCount; ++i)
					{
						if (GetPreviewScene(i)->GetJObject()->GetGuid() == dir->GetGuid())
						{
							hasOverlap = true;
							break;
						}
					}
					if (hasOverlap)
						continue;
				}
				if (canCreatePreview && isValidIndex)
					newPreviewScene.push_back(CreatePreviewScene(dir)); 
			}
			if (existPreviewCount > 0)
			{
				AlignByType(J_OBJECT_TYPE::RESOURCE_OBJECT);
				AlignByName();
			}
			pageCounter->EndCounting();
			return newPreviewScene;
		}
		JEditorPreviewInterface::JEditorPreviewInterface()
		{
			impl = std::make_unique<JEditorPreviewImpl>();
		}
		JEditorPreviewInterface::~JEditorPreviewInterface()
		{ 
			if (impl != nullptr)
				impl->ClearPreviewGroup();
			impl = nullptr; 
		}
	}
}