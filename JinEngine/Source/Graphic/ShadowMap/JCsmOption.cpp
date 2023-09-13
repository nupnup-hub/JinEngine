#include"JCsmOption.h" 
#include<algorithm>

namespace JinEngine::Graphic
{ 
	bool JCsmOption::operator=(const JCsmOption& rhs)const noexcept
	{
		bool isEqual = true;
		isEqual &= (splitBlendRate == rhs.splitBlendRate);
		isEqual &= (splitCount == rhs.splitCount);
		isEqual &= (shadowDistance == rhs.shadowDistance);
		isEqual &= (mapMinBorder == rhs.mapMinBorder);
		isEqual &= (mapMaxBorder == rhs.mapMaxBorder); 
		return isEqual;
	}
	uint JCsmOption::GetMaxSplitCount()const noexcept
	{
		return maxCountOfSplit;
	}
	uint JCsmOption::GetSplitCount()const noexcept
	{
		return splitCount;
	} 
	float JCsmOption::GetMapMinBorder()const noexcept
	{
		return mapMinBorder;
	}
	float JCsmOption::GetMapMaxBorder()const noexcept
	{
		return mapMaxBorder;
	}
	float JCsmOption::GetSplitBlendRate()const noexcept
	{
		return splitBlendRate;
	}
	float JCsmOption::GetShadowDistance()const noexcept
	{
		return shadowDistance;
	}
	void JCsmOption::SetSplitCount(const uint newCount)noexcept
	{
		splitCount = std::clamp(newCount, minCountOfSplit, maxCountOfSplit); 
	}
	void JCsmOption::SetSplitBlendRate(const float value)noexcept
	{
		splitBlendRate = std::clamp(value, 0.0f, 1.0f);
	}
	void JCsmOption::SetShadowDistance(const float value)noexcept
	{
		shadowDistance = std::clamp(value, 0.0f, value);
	} 
	void JCsmOption::SetMapMinBorder(const float newValue)noexcept
	{
		mapMinBorder = std::clamp(newValue, 0.0f, mapMaxBorder);
	}
	void JCsmOption::SetMapMaxBorder(const float newValue)noexcept
	{
		mapMaxBorder = std::clamp(newValue, mapMinBorder, 1.0f);
	}
}