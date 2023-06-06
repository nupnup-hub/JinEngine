#include"JPreviewSceneGroup.h"
#include"JPreviewScene.h"
#include"JPreviewResourceScene.h" 
#include"JPreviewDirectory.h"
#include"../JScene.h" 
#include"../../../Directory/JDirectory.h"
#include"../../../../Core/Guid/JGuidCreator.h"
#include"../../../../Graphic/Upload/Frameresource/JAnimationConstants.h"  
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
			previewSceneList[i].reset();
		}
		previewSceneList.clear();
		previewSceneList.shrink_to_fit();
	}
	JPreviewScene* JPreviewSceneGroup::CreatePreviewScene(_In_ JUserPtr<JObject> jObj, const J_PREVIEW_DIMENSION previewDimension, const J_PREVIEW_FLAG previewFlag)noexcept
	{
		if (!jObj.IsValid())
			return nullptr;

		if (previewSceneList.size() >= maxCapacity)
			return nullptr;

		JPreviewScene* res = nullptr;
		const uint previewSceneCount = (uint)previewSceneList.size();

		if (jObj->GetObjectType() == J_OBJECT_TYPE::RESOURCE_OBJECT)
		{
			JUserPtr<JResourceObject> resource;
			resource.ConnnectChild(jObj); 
			std::unique_ptr<JPreviewResourceScene> previewResouce = std::make_unique<JPreviewResourceScene>(resource, previewDimension, previewFlag);
			if (previewResouce->Initialze())
			{
				res = previewResouce.get();
				previewSceneList.push_back(std::move(previewResouce));
			}
		}
		else if (jObj->GetObjectType() == J_OBJECT_TYPE::DIRECTORY_OBJECT)
		{
			JUserPtr<JDirectory> dir;
			dir.ConnnectChild(jObj);		 
			std::unique_ptr<JPreviewDirectory> previewResouce = std::make_unique<JPreviewDirectory>(dir, previewDimension, previewFlag);
			if (previewResouce->Initialze())
			{
				res = previewResouce.get();
				previewSceneList.push_back(std::move(previewResouce));
			}
		}
		return res;
	}
	bool JPreviewSceneGroup::DestroyPreviewScene(JPreviewScene* previewScene)noexcept
	{
		const size_t sceneGuid = previewScene->GetGuid();
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
	bool JPreviewSceneGroup::DestroyPreviewScene(JUserPtr<JObject> jObj)noexcept
	{
		if (!jObj.IsValid())
			return false;

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
	void JPreviewSceneGroup::DestroyInvalidPreviewScene()noexcept
	{
		std::vector<std::unique_ptr<JPreviewScene>> newVector;
		for (uint i = 0; i < previewSceneList.size(); ++i)
		{
			if (previewSceneList[i]->GetJObject().IsValid())
				newVector.push_back(std::move(previewSceneList[i]));
		}
		previewSceneList = std::move(newVector);
	}
}
/*
std::wstring JPreviewSceneGroup::GetPreviewScenePath(const std::wstring& groupName, const std::wstring& resourcePath)noexcept
	{
		std::wstring resourceFolderPath;
		std::wstring resourceName;
		std::wstring resourceFormat;
		JCUtil::DecomposeFilePath(resourcePath, resourceFolderPath, resourceName, resourceFormat);

		return groupName + L"//" + resourceFolderPath + resourceName + L"##Editor_Preview_Scene" + resourceFormat + L".JPreviewScene";
	}
*/