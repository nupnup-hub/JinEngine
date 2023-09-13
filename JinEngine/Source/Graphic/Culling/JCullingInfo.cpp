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
		uint JCullingInfo::GetResultBufferSize()const noexcept
		{
			return resultHolder->GetBuffSize();
		}
		J_CULLING_TYPE JCullingInfo::GetCullingType()const noexcept
		{
			return cullingType;
		}  
		float JCullingInfo::GetUpdateFrequency()const noexcept
		{
			return updateFrequency;
		}
		float JCullingInfo::GetUpdatePerObjectRate()const noexcept
		{
			return updatePerObjectRate;
		}
		void JCullingInfo::SetArrayIndex(const int newValue)noexcept
		{
			arrayIndex = newValue;
		} 
		void JCullingInfo::SetUpdateFrequency(const bool value)noexcept
		{
			updateFrequency = value;
		}
		void JCullingInfo::SetUpdatePerObjectRate(const bool value)noexcept
		{
			updatePerObjectRate = value;
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
		JCullingInfo::JCullingInfo(JCullingManager* manager, const J_CULLING_TYPE cullingType, std::unique_ptr<JCullingResultHolder>&& resultHolder)
			:manager(manager), 
			cullingType(cullingType),
			resultHolder(std::move(resultHolder))
		{ }
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