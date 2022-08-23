#include"JPreviewSceneGroup.h"
#include"JPreviewScene.h"
#include"JPreviewResourceScene.h"
#include"JPreviewModelScene.h" 
#include"JPreviewDirectory.h"
#include"../JScene.h" 
#include"../../../Directory/JDirectory.h"
#include"../../../../Core/Guid/GuidCreator.h"
#include"../../../../Graphic/FrameResource/JAnimationConstants.h"  
#include"../../../../Utility/JCommonUtility.h"

namespace JinEngine
{
	JPreviewSceneGroup::JPreviewSceneGroup()
		:guid(Core::MakeGuid())
	{
		maxCapacity = initMaxCapacity;
	}
	JPreviewSceneGroup::~JPreviewSceneGroup() {}
	size_t JPreviewSceneGroup::GetGuid()const noexcept
	{
		return guid;
	}
	uint JPreviewSceneGroup::GetPreviewSceneCount()const noexcept
	{
		return (uint)previewSceneList.size();
	}
	JPreviewScene* JPreviewSceneGroup::GetPreviewScene(const uint index)const noexcept
	{
		if (index >= previewSceneList.size())
			return nullptr;

		return previewSceneList[index].get();
	}
	void JPreviewSceneGroup::SetCapacity(const uint value)noexcept
	{
		maxCapacity = value;
	}
	void JPreviewSceneGroup::Clear()noexcept
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
	JPreviewScene* JPreviewSceneGroup::CreatePreviewScene(_In_ JObject* jobject, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag)noexcept
	{
		if (previewSceneList.size() == maxCapacity)
			return nullptr;

		JPreviewScene* res = nullptr;
		const uint previewSceneCount = (uint)previewSceneList.size();

		if (jobject->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
		{
			JResourceObject* resource = static_cast<JResourceObject*>(jobject);
			if (resource->GetResourceType() == J_RESOURCE_TYPE::MODEL)
			{
				std::unique_ptr<JPreviewModelScene> previewModel = std::make_unique<JPreviewModelScene>(resource, previewDimension, previewFlag);
				if (previewModel->Initialze())
				{
					res = previewModel.get();
					previewSceneList.push_back(std::move(previewModel));
				}
			}
			else
			{
				std::unique_ptr<JPreviewResourceScene> previewResouce = std::make_unique<JPreviewResourceScene>(resource, previewDimension, previewFlag);
				if (previewResouce->Initialze())
				{
					res = previewResouce.get();
					previewSceneList.push_back(std::move(previewResouce));
				}
			}
		}
		else if (jobject->GetObjectType() == J_OBJECT_TYPE::DIRECTORY_OBJECT)
		{
			JDirectory* jDir = static_cast<JDirectory*>(jobject);
			std::unique_ptr<JPreviewDirectory> previewResouce = std::make_unique<JPreviewDirectory>(jDir, previewDimension, previewFlag);
			if (previewResouce->Initialze())
			{
				res = previewResouce.get();
				previewSceneList.push_back(std::move(previewResouce));
			}
		}
		return res;
	}
	bool JPreviewSceneGroup::DestroyPreviewScene(const size_t sceneGuid)noexcept
	{
		const uint previewCount = (uint)previewSceneList.size();
		for (uint i = 0; i < previewCount; ++i)
		{
			if (sceneGuid == previewSceneList[i]->GetGuid())
			{
				previewSceneList[i]->Clear();
				previewSceneList.erase(previewSceneList.begin() + i);
				return true;
			}
		}
		return false;
	}
	bool JPreviewSceneGroup::DestroyPreviewScene(JObject* jObj)noexcept
	{
		const size_t resourceGuid = jObj->GetGuid();
		const uint previewCount = (uint)previewSceneList.size();
		for (uint i = 0; i < previewCount; ++i)
		{
			if (resourceGuid == previewSceneList[i]->GetJObject()->GetGuid())
			{
				previewSceneList[i]->Clear();
				previewSceneList.erase(previewSceneList.begin() + i);
				return true;
			}
		}
		return false;
	}
}
/*
std::wstring JPreviewSceneGroup::GetPreviewScenePath(const std::wstring& groupName, const std::wstring& resourcePath)noexcept
	{
		std::wstring resourceFolderPath;
		std::wstring resourceName;
		std::wstring resourceFormat;
		JCommonUtility::DecomposeFilePath(resourcePath, resourceFolderPath, resourceName, resourceFormat);

		return groupName + L"//" + resourceFolderPath + resourceName + L"##Editor_Preview_Scene" + resourceFormat + L".JPreviewScene";
	}
*/