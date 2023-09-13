#include"JEditorDynamicColor.h"
#include"../../Core/Math/JVectorExtend.h" 
#include"../../Core/Math/JMathHelper.h" 
#include<algorithm>
#include<Windows.h>

namespace JinEngine::Editor
{
	JVector4F JEditorDynamicSpotColor::GetColor(const int index)const noexcept
	{
		if (index >= point.size())
			return JVector4F::Zero();
		 
		const float xFactor = cosf(updateFactor * JMathHelper::Pi);
		const float yFactor = sinf(updateFactor * JMathHelper::Pi);
  
		JVector4F resultCol = color + deltaColor * xFactor * point[index].x + deltaColor * yFactor * point[index].y;
		if (highLightRange > 0)
		{
			float highLightLeftFactor = highLightPoint - highLightRange;
			float highLightRightFactor = highLightPoint + highLightRange;
			float minHighLightFactor = min(highLightLeftFactor, highLightRightFactor);
			float maxHighLightFactor = max(highLightLeftFactor, highLightRightFactor);
			  
			float rate = 0;
			if (minHighLightFactor <= updateFactor && updateFactor <= maxHighLightFactor)
				rate = 1 - (abs(highLightPoint - updateFactor) / highLightRange);
			else
			{
				//range -1 ~ 1을 초과했을시
				//조정된 range값에 updateFactor가 위치하는지 검사
				const float scaledOverMinFactor = 1.0f + (minHighLightFactor + 1.0f);
				const float scaledOverMaxFactor = -1.0f + (maxHighLightFactor - 1.0f);
				if (minHighLightFactor < -1.0f && updateFactor >= scaledOverMinFactor)
					rate = (abs(updateFactor - scaledOverMinFactor) / highLightRange);
				else if (maxHighLightFactor > 1.0f && updateFactor <= scaledOverMaxFactor)
					rate = (abs(scaledOverMaxFactor - updateFactor) / highLightRange);
			} 
			if (rate > 0)
				resultCol += rate * highLightColor;
		}
		return resultCol;
	}
	uint JEditorDynamicSpotColor::GetPointCount()const noexcept
	{
		return (uint)point.size();
	} 
	void JEditorDynamicSpotColor::SetHighLight(const JVector4F& newHighlightColor,
		const float newHighLightPoint,
		const float newHighLightRange)
	{
		highLightColor = newHighlightColor;
		highLightPoint = std::clamp(newHighLightPoint, -1.0f, 1.0f);
		highLightRange = std::clamp(newHighLightRange, 0.0f, 1.0f);
	}
	void JEditorDynamicSpotColor::Update()
	{
		updateFactor += updateSpeed;
		if (updateFactor > 1)
			updateFactor = -1;
		/*
		
		const float xFactor = cosf(updateFactor * JMathHelper::Pi);
		const float yFactor = sinf(updateFactor * JMathHelper::Pi);

		if (highLightRange > 0)
		{
			float highLightLeftFactor = highLightPoint - highLightRange;
			float highLightRightFactor = highLightPoint + highLightRange;
			float minHighLightFactor = min(highLightLeftFactor, highLightRightFactor);
			float maxHighLightFactor = max(highLightLeftFactor, highLightRightFactor);

			if (minHighLightFactor <= updateFactor && updateFactor <= maxHighLightFactor)
			{
				const float rate = 1 - (abs((highLightPoint - updateFactor)) / highLightRange);
				//MessageBoxA(0, (std::to_string((highLightPoint - updateFactor)) + " " + std::to_string(rate)).c_str(),
				//	(std::to_string(highLightPoint) + " " +
				//		std::to_string(updateFactor) + " " +
				//		std::to_string(highLightRange)).c_str(), 0);
			};		 
		}
		*/
	}
	bool JEditorDynamicSpotColor::PushPoint(JVector2F scaledPoint)
	{
		point.push_back(ClampVec2(scaledPoint, -1.0f, 1.0f));
		return true;
	}
	bool JEditorDynamicSpotColor::PushPoint(JVector2F unScaledPoint, const JVector2F& maxPoint)
	{
		point.push_back(ClampVec2((unScaledPoint / maxPoint) * 2 - JVector2F(1.0f, 1.0f), -1.0f, 1.0f));
		return true;
	}
	bool JEditorDynamicSpotColor::PopPoint()
	{
		point.pop_back();
		return true;
	}
	void JEditorDynamicSpotColor::Clear()
	{
		point.clear();
	}
}