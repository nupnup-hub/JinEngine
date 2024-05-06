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


#include"JCullingInfo.h"
#include"JCullingConstants.h"
#include"JCullingResultHolder.h"
#include"../JGraphicPrivate.h"
#include"../../Core/Guid/JGuidCreator.h"
#include"../../Core/Time/JGameTimer.h" 

namespace JinEngine
{
	namespace Graphic
	{
		int JCullingInfo::GetArrayIndex()const noexcept
		{
			return arrayIndex;
		} 
		uint JCullingInfo::GetResultBufferElementCount()const noexcept
		{
			return resultHolder->GetElementCount();
		}
		uint JCullingInfo::GetResultBufferSize()const noexcept
		{
			return resultHolder->GetBufferSize();
		}
		uint JCullingInfo::GetUpdatedInfoCount()const noexcept
		{
			return (uint)updatedInfo.size();
		}
		JCullingUpdatedInfo JCullingInfo::GetUpdateddInfo(const uint index)const noexcept
		{
			return updatedInfo[index];
		}
		J_CULLING_TYPE JCullingInfo::GetCullingType()const noexcept
		{
			return cullingType;
		}  
		J_CULLING_TARGET JCullingInfo::GetCullingTarget()const noexcept
		{
			return resultHolder->GetCullingTarget();
		}
		float JCullingInfo::GetUpdateFrequency()const noexcept
		{
			return updateFrequency;
		} 
		void JCullingInfo::SetArrayIndex(const int newValue)noexcept
		{
			arrayIndex = newValue;
		} 
		void JCullingInfo::SetUpdateFrequency(const bool value)noexcept
		{
			updateFrequency = value;
		} 
		void JCullingInfo::SetUpdatedInfo(const JCullingUpdatedInfo& info, const uint index)noexcept
		{
			updatedInfo[index] = info;
		}
		void JCullingInfo::SetUpdateEnd(const bool value)noexcept
		{
			isUpdateEnd = value;
		}
		void JCullingInfo::Culling(const uint index, const bool value)const noexcept
		{  
			resultHolder->Culling(index, value);
		}
		bool JCullingInfo::IsCulled(const uint index)const noexcept
		{ 
			return resultHolder->IsCulled(index);
		}   
		bool JCullingInfo::IsCullingResultInGpu()const noexcept
		{
			return resultHolder->IsGpuResource();
		}
		bool JCullingInfo::IsUpdateEnd()const noexcept
		{
			return isUpdateEnd;
		}
		bool JCullingInfo::CanSetCullingValue()const noexcept
		{
			return resultHolder->CanSetValue();
		}
		bool JCullingInfo::Destroy(JCullingInfo* info)
		{
			return JGraphicPrivate::CullingInterface::DestroyCullignData(info);;
		}
		JCullingInfo::JCullingInfo(JCullingManager* manager,
			const J_CULLING_TYPE cullingType,
			const uint updatedInfoCount,
			std::unique_ptr<JCullingResultHolder>&& resultHolder)
			:manager(manager), 
			cullingType(cullingType),
			resultHolder(std::move(resultHolder))
		{
			updatedInfo.resize(updatedInfoCount);
		}
		JCullingInfo::~JCullingInfo()
		{
			resultHolder = nullptr;
		}
	}
}

/*
		void JCullingInfo::SetUpdateFrequency(const float newUpdateFrequency, const float startTime)noexcept
		{
			updateFrequency = std::clamp(newUpdateFrequency, Constants::cullingUpdateFrequencyMin, Constants::cullingUpdateFrequencyMax);
			elapsedTime = startTime;
		}
		void JCullingInfo::Update()noexcept
		{
			canCulling = false;
			elapsedTime += JEngineTimer::Data().DeltaTime();
			if (elapsedTime >= updateFrequency)
			{
				canCulling = true;
				elapsedTime = 0;
			}
		}
		bool JCullingInfo::CanCulling()const noexcept
		{
			return canCulling;
		}
*/