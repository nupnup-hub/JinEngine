#pragma once
#include"PreviewEnum.h" 
#include"../../../../Core/JDataType.h"
#include<vector>
#include<string> 
#include<memory> 

namespace JinEngine
{ 
	class JResourceObject;
	class PreviewScene;
	class JGameObject;
	class JMaterial; 
	class JScene; 
	/*
	* PreviewScene is Managed Editor
	*/
	class PreviewSceneGroup
	{ 
		friend class JResourceManager;
	public:
		constexpr static int initMaxCapacity = 50;
	private:   
		const std::string name;
		size_t guid;
		uint maxCapacity;  
		std::vector<std::unique_ptr<PreviewScene>> previewSceneList;  
	public:
		PreviewSceneGroup(const std::string& ownerName, const uint maxCapacity);
		~PreviewSceneGroup();
		PreviewSceneGroup(PreviewSceneGroup& rhs) = delete;
		PreviewSceneGroup& operator=(PreviewSceneGroup& rhs) = delete;

		size_t GetGuid()const noexcept; 
		std::string GetGroupName()const noexcept;
		uint GetPreviewSceneCount()const noexcept;  
		JResourceObject* GetResouceObject(const uint index)const noexcept; 	
		PreviewScene* GetPreviewScene(const uint index)const noexcept;
		//static std::wstring GetPreviewScenePath(const std::wstring& groupName, const std::wstring& resourcePath)noexcept;
	private:
		PreviewScene* CreateResourcePreviewScene(_In_ JResourceObject* resource, const PREVIEW_DIMENSION previewDimension, const PREVIEW_FLAG previewFlag)noexcept;
		void Clear()noexcept;
		bool Erase(JResourceObject* resource)noexcept; 
		std::string MakePreviewSceneName(const uint num)noexcept;
	};
}