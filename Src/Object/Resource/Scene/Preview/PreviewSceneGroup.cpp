#include"PreviewSceneGroup.h"
#include"PreviewScene.h"
#include"PreviewResourceScene.h"
#include"PreviewModelScene.h" 
#include"../JScene.h" 
#include"../../../../Graphic/FrameResource/JAnimationConstants.h"  
#include"../../../../Utility/JCommonUtility.h"

namespace JinEngine
{
	PreviewSceneGroup::PreviewSceneGroup(const std::string& ownerName, const uint maxCapacity)
		:name(ownerName + "_PreviewSceneGroup")
	{
		guid = JCommonUtility::CalculateGuid(name); 
		if (maxCapacity == 0)
			PreviewSceneGroup::maxCapacity = initMaxCapacity;
		else
			PreviewSceneGroup::maxCapacity = maxCapacity; 
	}
	PreviewSceneGroup::~PreviewSceneGroup() {}
	PreviewScene* PreviewSceneGroup::CreateResourcePreviewScene(_In_ JResourceObject* resource, const PREVIEW_DIMENSION previewDimension, const PREVIEW_FLAG previewFlag)noexcept
	{
		if (previewSceneList.size() == maxCapacity)
			return nullptr;

		PreviewScene* res = nullptr;
		const uint previewSceneCount = (uint)previewSceneList.size();
		std::string previewSceneName = MakePreviewSceneName(previewSceneCount);

		if (resource->GetResourceType() == J_RESOURCE_TYPE::MODEL)
		{
			std::unique_ptr<PreviewModelScene> previewModel = 
				std::make_unique<PreviewModelScene>(previewSceneName, resource, previewDimension, previewFlag);
			if (previewModel->Initialze())
			{
				res = previewModel.get();
				previewSceneList.push_back(std::move(previewModel));
			}
		}
		else
		{
			std::unique_ptr<PreviewResourceScene> previewResouce = 
				std::make_unique<PreviewResourceScene>(previewSceneName, resource, previewDimension, previewFlag);
			if (previewResouce->Initialze())
			{
				res = previewResouce.get();
				previewSceneList.push_back(std::move(previewResouce)); 
			}
		}
		return res;
	}
	size_t PreviewSceneGroup::GetGuid()const noexcept
	{
		return guid;
	}
	std::string PreviewSceneGroup::GetGroupName()const noexcept
	{
		return name;
	}
	uint PreviewSceneGroup::GetPreviewSceneCount()const noexcept
	{
		return (uint)previewSceneList.size();
	}
	JResourceObject* PreviewSceneGroup::GetResouceObject(const uint index)const noexcept
	{
		if (index >= previewSceneList.size())
			return nullptr;

		return previewSceneList[index]->GetResouceObject();
	}
	PreviewScene* PreviewSceneGroup::GetPreviewScene(const uint index)const noexcept
	{
		if (index >= previewSceneList.size())
			return nullptr;

		return previewSceneList[index].get();
	}
	void PreviewSceneGroup::Clear()noexcept
	{
		const uint previewCount = (uint)previewSceneList.size();
		for (uint i = 0; i < previewCount; ++i)
		{ 
			previewSceneList[i]->Clear();
			previewSceneList[i].release();
		}
		previewSceneList.clear();
		previewSceneList.shrink_to_fit();
	}
	bool PreviewSceneGroup::Erase(JResourceObject* resource)noexcept
	{
		const size_t guid = resource->GetGuid();
		const uint previewCount = (uint)previewSceneList.size();
		for (uint i = 0; i < previewCount; ++i)
		{
			if (guid == previewSceneList[i]->GetResouceObject()->GetGuid())
			{ 
				previewSceneList[i]->Clear();
				previewSceneList.erase(previewSceneList.begin() + i);
				return true;
			}
		}
		return false;
	}
	std::string PreviewSceneGroup::MakePreviewSceneName(const uint num)noexcept
	{
		return name + "_PreviewScene_No: " + std::to_string(num);
	}
}
/*
std::wstring PreviewSceneGroup::GetPreviewScenePath(const std::wstring& groupName, const std::wstring& resourcePath)noexcept
	{
		std::wstring resourceFolderPath;
		std::wstring resourceName;
		std::wstring resourceFormat;
		JCommonUtility::DecomposeFilePath(resourcePath, resourceFolderPath, resourceName, resourceFormat);

		return groupName + L"//" + resourceFolderPath + resourceName + L"##Editor_Preview_Scene" + resourceFormat + L".PreviewScene";
	}
*/