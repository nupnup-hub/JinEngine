#pragma once
#include"JPreviewEnum.h" 
#include"../../../../Core/JDataType.h"
#include"../../../../Core/Pointer/JOwnerPtr.h"
#include<vector>
#include<string> 
#include<memory> 

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
		constexpr static int initMaxCapacity = 250;
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
		void SetCapacity(const uint value)noexcept;
	private:
		void Clear()noexcept;
	private:
		JPreviewScene* CreatePreviewScene(_In_ JUserPtr<JObject> jObj, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag)noexcept;
		bool DestroyPreviewScene(JPreviewScene* previewScene)noexcept;
		bool DestroyPreviewScene(JUserPtr<JObject> jObj)noexcept;
		void DestroyInvalidPreviewScene()noexcept;
	};
}