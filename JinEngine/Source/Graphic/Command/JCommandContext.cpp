#include"JCommandContext.h"
#include"../JGraphicConstants.h"
#include"../../Core/Time/JGameTimer.h"
//#include"../../Application/Engine/JApplicationEngine.h"
#include<Windows.h>

namespace JinEngine::Graphic
{  
	JCommandContext::JCommandContext(const std::string& name, const bool isAlwausActivated)
		:name(name), isAlwausActivated(isAlwausActivated)
	{
		log.name = name;
	}
	bool JCommandContext::Begin()
	{
		if (!canUse)
			return false;
		isLastFrameUpdated = true; 	
#ifdef _DEBUG
		if (canWriteLog)
			stopWatch.Reset();
#endif
		return true;
	}
	void JCommandContext::End()
	{
		uint logUpdateBorder = Core::JGameTimer::FramePerSecond() * 2.5f;
#ifdef _DEBUG
		if (canWriteLog)
		{
			stopWatch.Stop(); 
			if (log.lastUpdatedMsTime == 0 || logUpdateCount >= logUpdateBorder)
			{
				log.lastUpdatedMsTime = stopWatch.GetElapsedMilliTime();
				logUpdateCount = 0;
			}
			else
				++logUpdateCount;
		}
#endif
	} 
	JCommandContextLog JCommandContext::GetContextLog()const noexcept
	{
		return log;
	}
	void JCommandContext::SetUseTrigger(const bool value)
	{
		canUse = value;
	}
	void JCommandContext::SetWriteLogTrigger(const bool value)
	{
		canWriteLog = value;
	}
	bool JCommandContext::CanUse()const noexcept
	{
		return canUse;
	} 

	JCommandContextManager::~JCommandContextManager()
	{
		Clear();
	}
	std::vector<JCommandContextLog> JCommandContextManager::GetLog()
	{
		std::vector<JCommandContextLog> result;
		result.resize(context.size()); 
		 
		for (uint i = 0; i < context.size(); ++i)
			result[i] = context[i]->GetContextLog();
		return result;
	}
	JCommandContext* JCommandContextManager::PushCommandContext(std::unique_ptr<JCommandContext>&& newContext)
	{
		if (newContext == nullptr)
			return nullptr;

		context.push_back(std::move(newContext));
		return context[(uint)context.size() - 1].get();
	}
	void JCommandContextManager::Begin()
	{
		for (auto& data : context)
		{
			data->canUse = true;
			data->isLastFrameUpdated = false;
		}
	}
	void JCommandContextManager::End()
	{
		for (auto& data : context)
		{
			if (!data->isAlwausActivated)
				data->canUse = false;
		}
	}
	void JCommandContextManager::Clear()
	{
		context.clear();
	} 
}