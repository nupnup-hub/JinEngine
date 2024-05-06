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