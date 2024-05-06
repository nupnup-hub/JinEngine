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
	JCsmManager::~JCsmManager()
	{
		Clear();
	}
	bool JCsmManager::RegisterHandler(JCsmHandlerInterface* handler)
	{
		auto existHandle = GetHandler(handler->GetCsmAreaGuid(), handler->GetCsmHandlerGuid());
		if (existHandle != nullptr)
			return false;

		auto area = areaData.find(handler->GetCsmAreaGuid());
		if (area == areaData.end())
		{
			areaData.emplace(handler->GetCsmAreaGuid(), AreaData());
			area = areaData.find(handler->GetCsmAreaGuid());
		}

		//handler->handlerIndex = (uint)area->second.handler.size();
		area->second.handler.push_back(handler->GetPointerWrapper());
		for (auto& data : area->second.target)
		{
			auto info = handler->CreateTargetInfo();
			if (info != nullptr)
				data->Get()->AddCsmTargetInfo(info);
		} 
		return true;
	}
	bool JCsmManager::DeRegisterHandler(JCsmHandlerInterface* handler)
	{
		int existHandlerIndex = GetHandlerIndex(handler->GetCsmAreaGuid(), handler->GetCsmHandlerGuid());
		if (existHandlerIndex == -1)
			return false;

		auto area = areaData.find(handler->GetCsmAreaGuid());
		if (area == areaData.end())
			return false;

		handler->DestroyAllTargetInfo(); 
		//const uint count = (uint)area->second.handler.size();
		//for (uint i = existHandlerIndex + 1; i < count; ++i)
		//	--area->second.handler[i]->Get()->handlerIndex;

		area->second.handler.erase(area->second.handler.begin() + existHandlerIndex);
		if (area->second.handler.size() == 0 && area->second.target.size() == 0)
			areaData.erase(handler->GetCsmAreaGuid());
		return true;
	}
	bool JCsmManager::RegisterTarget(JCsmTargetInterface* target)
	{
		auto existTarget = GetTarget(target->GetCsmAreaGuid(), target->GetCsmTargetGuid());
		if (existTarget != nullptr)
			return false;

		auto area = areaData.find(target->GetCsmAreaGuid());
		if (area == areaData.end())
		{
			areaData.emplace(target->GetCsmAreaGuid(), AreaData());
			area = areaData.find(target->GetCsmAreaGuid());
		}

		target->targetIndex = (uint)area->second.target.size();
		area->second.target.push_back(target->GetPointerWrapper());
		for (auto& data : area->second.handler)
		{
			auto info = data->Get()->CreateTargetInfo();
			if (info != nullptr)
			{
				target->AddCsmTargetInfo(info);
				data->Get()->NotifyAddCsmTarget(data->Get()->GetCsmTargetCount() - 1);
			}
		}
		return true;
	}
	bool JCsmManager::DeRegisterTarget(JCsmTargetInterface* target)
	{
		int existTargetIndex = GetTargetIndex(target->GetCsmAreaGuid(), target->GetCsmTargetGuid());
		if (existTargetIndex == -1)
			return false;

		auto area = areaData.find(target->GetCsmAreaGuid());
		if (area == areaData.end())
			return false;

		const uint handlerCount = (uint)area->second.handler.size();
		for (uint i = 0; i < handlerCount; ++i)
		{
			area->second.handler[i]->Get()->DestroyTargetInfo(target->GetCsmTargetIndex());
			area->second.handler[i]->Get()->NotifyPopCsmTarget(i);
		}
		
		const uint targetCount = (uint)area->second.target.size();
		for (uint i = existTargetIndex + 1; i < targetCount; ++i)
			--area->second.target[i]->Get()->targetIndex;
		area->second.target.erase(area->second.target.begin() + existTargetIndex);

		for (uint i = 0; i < handlerCount; ++i)
		{
			if(area->second.handler[i]->Get()->GetCsmTargetCount() == 0)
				area->second.handler[i]->Get()->NotifyCsmTargetZero();
		}
		if (area->second.handler.size() == 0 && area->second.target.size() == 0)
			areaData.erase(target->GetCsmAreaGuid());
		return true;
	}
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
	void JCsmManager::Clear()
	{
		areaData.clear();
	}
}