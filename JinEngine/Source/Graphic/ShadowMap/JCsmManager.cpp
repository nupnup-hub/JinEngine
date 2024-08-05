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


#include"JCsmManager.h"
#include"JCsmHandlerInterface.h"
#include"JCsmTargetInterface.h"
#include"../../Core/Utility/JCommonUtility.h"
#include"../../Core/Log/JLogMacro.h"

namespace JinEngine::Graphic
{
	JCsmManager::AreaData::AreaData(const size_t guid)
	{ 
		areaInfo.SetGuid(guid);
	}

	JCsmManager::~JCsmManager()
	{
		Clear();
	}
	JUserPtr<JCsmHandlerInfo> JCsmManager::CreateHandler(const JCsmHandleCreationDesc& desc)
	{
		auto newHandle = Core::JPtrUtil::MakeOwnerPtr<JCsmHandlerInfo>(std::move(desc.notifyAddCsmTargetB), std::move(desc.notifySubtractCsmTargetB));
		auto area = areaData.find(desc.areaGuid);
		if (area == areaData.end())
		{
			CreateAreaData(desc.areaGuid);
			area = areaData.find(desc.areaGuid);
		}

		CsmHandlerInfoVec& handlerVec = area->second.handler;
		CsmTargetInfoVec& targetVec = area->second.target;

		newHandle->SetIndex(handlerVec.size());
 
		const uint existTargetCount = (uint)targetVec.size();
		for (uint i = 0; i < existTargetCount; ++i)
			newHandle->AddTarget(targetVec[i]);

		JUserPtr<JCsmHandlerInfo> result = newHandle;
		handlerVec.push_back(std::move(newHandle));
		return result;
	}
	JUserPtr<JCsmTargetInfo> JCsmManager::CreateTarget(const JCsmTargetCreationDesc& desc)
	{
		auto newTarget = Core::JPtrUtil::MakeOwnerPtr<JCsmTargetInfo>(std::move(desc.getBoundingFrustumB));
		auto area = areaData.find(desc.areaGuid);
		if (area == areaData.end())
		{
			CreateAreaData(desc.areaGuid);
			area = areaData.find(desc.areaGuid); 
		}

		CsmHandlerInfoVec& handlerVec = area->second.handler;
		CsmTargetInfoVec& targetVec = area->second.target;

		newTarget->SetIndex(targetVec.size());

		const uint existHandleCount = (uint)handlerVec.size();
		for (uint i = 0; i < existHandleCount; ++i)
			handlerVec[i]->AddTarget(newTarget);

		JUserPtr<JCsmTargetInfo> result = newTarget;
		targetVec.push_back(std::move(newTarget));
		return result;
	}
	bool JCsmManager::DestroyHandler(JUserPtr<JCsmHandlerInfo>& data)
	{
		const size_t areaGuid = data->GetAreaInfo()->GetGuid();
		auto area = areaData.find(areaGuid);
		if (area == areaData.end())
		{ 
			J_LOG_PRINT_OUT_SIMPLE("Invalid Area Guid");
			return false;
		}

		CsmHandlerInfoVec& handlerVec = area->second.handler;
		CsmTargetInfoVec& targetVec = area->second.target;

		//Manage index
		const uint handleCount = (uint)handlerVec.size();
		for (uint i = data->GetIndex() + 1; i < handleCount; ++i)
			handlerVec[i]->SetIndex(handlerVec[i]->GetIndex() - 1);

		handlerVec.erase(handlerVec.begin() + data->GetIndex());
		data.Release();

		//Try to erase area data if 0 member
		if (handlerVec.size() == 0 && targetVec.size() == 0)
			areaData.erase(areaGuid);
		return true;
	}
	bool JCsmManager::DestroyTarget(JUserPtr<JCsmTargetInfo>& data)
	{
		const size_t areaGuid = data->GetAreaInfo()->GetGuid();
		auto area = areaData.find(areaGuid);
		if (area == areaData.end())
		{
			J_LOG_PRINT_OUT_SIMPLE("Invalid Area Guid");
			return false;
		}

		CsmHandlerInfoVec& handlerVec = area->second.handler;
		CsmTargetInfoVec& targetVec = area->second.target;

		//Manage handle target caching
		const uint handleCount = (uint)handlerVec.size();
		for (uint i = 0; i < handleCount; ++i)
			handlerVec[i]->RemoveTarget(data->GetIndex());

		//Manage index
		const uint targetCount = (uint)targetVec.size();
		for (uint i = data->GetIndex() + 1; i < targetCount; ++i)
			targetVec[i]->SetIndex(targetVec[i]->GetIndex() - 1);

		targetVec.erase(targetVec.begin() + data->GetIndex());
		data.Release();
	 
		//Try to erase area data if 0 member
		if (handlerVec.size() == 0 && targetVec.size() == 0)
			areaData.erase(areaGuid);
		return true;
	}  
	void JCsmManager::CreateAreaData(const size_t guid)
	{ 
		areaData.emplace(guid, AreaData(guid));
	} 
	void JCsmManager::Clear()
	{
		areaData.clear();
	}
}
/*	unuse
	JUserPtr<JCsmHandlerPointer> JCsmManager::GetHandler(const size_t areaGuid, const size_t handlerGuid)const noexcept
	{
		auto area = areaData.find(areaGuid);
		if (area == areaData.end())
			return nullptr;

		auto& handlerVec = area->second.handler;
		const uint count = (uint)handlerVec.size();
		for (uint i = 0; i < count; ++i)
		{
			if (handlerVec[i]->Get()->GetCsmHandlerGuid() == handlerGuid)
				return handlerVec[i];
		}
		return nullptr;
	}
	JUserPtr<JCsmTargetInterfacePointer> JCsmManager::GetTarget(const size_t areaGuid, const size_t targetGuid)const noexcept
	{
		auto area = areaData.find(areaGuid);
		if (area == areaData.end())
			return nullptr;

		auto& targetVec = area->second.target;
		const uint count = (uint)targetVec.size();
		for (uint i = 0; i < count; ++i)
		{
			if (targetVec[i]->Get()->GetCsmTargetGuid() == targetGuid)
				return targetVec[i];
		}
		return nullptr;
	}
	int JCsmManager::GetHandlerIndex(const size_t areaGuid, const size_t handlerGuid)const
	{
		auto area = areaData.find(areaGuid);
		if (area == areaData.end())
			return -1;

		auto& handlerVec = area->second.handler;
		const uint count = (uint)handlerVec.size();
		for (uint i = 0; i < count; ++i)
		{
			if (handlerVec[i]->Get() == nullptr)
				J_LOG_PRINT_OUT("Null csm handler", "Count: " + std::to_string(count) + " Index: " + std::to_string(i));
			if (handlerVec[i]->Get()->GetCsmHandlerGuid() == handlerGuid)
				return i;
		}
		return -1;
	}
	int JCsmManager::GetTargetIndex(const size_t areaGuid, const size_t targetGuid)const noexcept
	{
		auto area = areaData.find(areaGuid);
		if (area == areaData.end())
			return -1;

		auto& targetVec = area->second.target;
		const uint count = (uint)targetVec.size();
		for (uint i = 0; i < count; ++i)
		{
			if (targetVec[i]->Get()->GetCsmTargetGuid() == targetGuid)
				return i;
		}
		return -1;
	}
*/