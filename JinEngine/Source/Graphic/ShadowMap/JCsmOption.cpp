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
		isEqual &= (levelBlendRate == rhs.levelBlendRate);
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
	float JCsmOption::GetSplitBlendRate()const noexcept
	{
		return splitBlendRate;
	}
	float JCsmOption::GetShadowDistance()const noexcept
	{
		return shadowDistance;
	}
	float JCsmOption::GetLevelBlendRate()const noexcept
	{
		return levelBlendRate;
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
	void JCsmOption::SetLevelBlendRate(const float value)noexcept
	{
		levelBlendRate = std::clamp(value, 0.0f, 1.0f);
	} 
}