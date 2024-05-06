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
#include"JPreviewEnum.h" 
#include"../../../JObjectType.h"
#include"../../../../Core/JCoreEssential.h"
#include"../../../../Core/Pointer/JOwnerPtr.h" 

namespace JinEngine
{  
	class JPreviewScene;
	class JGameObject;
	class JMaterial; 
	class JScene; 
	class JObject;

	namespace Editor
	{
		class JEditorPreviewImpl;
	}
	/*
	* JPreviewScene is Managed Editor
	*/
	class JPreviewSceneGroup
	{ 
	private:
		friend class Editor::JEditorPreviewImpl;
	public:
		constexpr static int initMaxCapacity = 1000;
	private:    
		const size_t guid;
		uint maxCapacity;  
		std::vector<std::unique_ptr<JPreviewScene>> previewSceneList;  
	public:
		JPreviewSceneGroup();
		~JPreviewSceneGroup();
		JPreviewSceneGroup(JPreviewSceneGroup& rhs) = delete;
		JPreviewSceneGroup& operator=(JPreviewSceneGroup& rhs) = delete;
	public:
		size_t GetGuid()const noexcept;   
		uint GetPreviewSceneCount()const noexcept;   
		JPreviewScene* GetPreviewScene(const uint index)const noexcept; 
	public:
		void SetCapacity(const uint value)noexcept;
	private:
		void Clear()noexcept;
	private:
		JPreviewScene* CreatePreviewScene(_In_ JUserPtr<JObject> jObj, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag)noexcept;
		bool DestroyPreviewScene(JPreviewScene* previewScene)noexcept;
		bool DestroyPreviewScene(JUserPtr<JObject> jObj)noexcept;
		void DestroyInvalidPreviewScene()noexcept;
		bool PopPreviewScene()noexcept;
	public:
		void AlignByName(const bool isAscending)noexcept;
		void AlignByType(const J_OBJECT_TYPE type)noexcept;
	};
}