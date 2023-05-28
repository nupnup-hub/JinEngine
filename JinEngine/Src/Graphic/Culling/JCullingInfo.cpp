#include"JCullingInfo.h"
#include"JCullingConstants.h"
#include"../JGraphicPrivate.h"
#include"../../Core/Guid/GuidCreator.h"
#include"../../Core/Time/JGameTimer.h"

namespace JinEngine
{
	namespace Graphic
	{
		int JCullingInfo::GetArrayIndex()const noexcept
		{
			return arrayIndex;
		}
		size_t JCullingInfo::GetResultSize()const noexcept
		{
			return resultSize;
		}
		J_CULLING_TYPE JCullingInfo::GetCullingType()const noexcept
		{
			return cullingType;
		}  
		void JCullingInfo::SetArrayIndex(const int newValue)noexcept
		{
			arrayIndex = newValue;
		}
		void JCullingInfo::SetUpdateFrequency(const float newUpdateFrequency, const float startTime)noexcept
		{
			updateFrequency = std::clamp(newUpdateFrequency, Constant::cullingUpdateFrequencyMin, Constant::cullingUpdateFrequencyMax);
			elapsedTime = startTime;
		} 
		void JCullingInfo::SetResultPtr(void* ptr, const size_t size)noexcept
		{
			result = ptr;
			resultSize = size;
			cullingPtr = JCullingTypeData::GetCullingPtr(cullingType);
			isCullingPtr = JCullingTypeData::GetIsCullingPtr(cullingType); 
		}
		void JCullingInfo::Culling(const uint index, const bool value)const noexcept
		{
			(*cullingPtr)(result, index, value);
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
		bool JCullingInfo::IsCulled(const uint index)const noexcept
		{
			//result && isCullingPtr은 항상 유효한 데이터를 가진다
			//if (result == nullptr || isCullingPtr == nullptr)
			//	return false;
			return (*isCullingPtr)(result, index);
		}
		bool JCullingInfo::UnsafeIsCulled(const uint index)const noexcept
		{
			return (*isCullingPtr)(result, index);
		}
		bool JCullingInfo::CanCulling()const noexcept
		{
			return canCulling;
		} 
		bool JCullingInfo::Destroy(JCullingInfo* info)
		{
			return JGraphicPrivate::CullingInterface::DestroyCullignData(info);;
		}
		JCullingInfo::JCullingInfo(JCullingManager* manager, const J_CULLING_TYPE cullingType)
			:manager(manager), cullingType(cullingType)
		{}
		JCullingInfo::~JCullingInfo(){ }
	}
}