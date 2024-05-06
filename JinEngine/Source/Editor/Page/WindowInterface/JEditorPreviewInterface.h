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
#include"../../../Object/Resource/Scene/Preview/JPreviewEnum.h" 
#include"../../../Object/JObjectType.h"
#include"../../../Core/JCoreEssential.h"
#include"../../../Core/Pointer/JOwnerPtr.h"
#include"../../../Core/Reflection/JTypeInfo.h" 

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