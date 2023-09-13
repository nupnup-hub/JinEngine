#pragma once
#include"../../Core/Math/JVector.h"
#include"../../Core/Utility/JMacroUtility.h"
#include<algorithm>

namespace JinEngine
{
	namespace Editor
	{
		class JEditorDynamicSpotColor
		{
		private:
			static constexpr float minUpdateSpeed = 0.00001f;
			static constexpr float maxUpdateSpeed = 0.1f;
		private:
			std::vector<JVector2F> point; 
		private:
			JVector4F color = JVector4F::Zero();
			JVector4F deltaColor = JVector4F::Zero();
			JVector4F highLightColor = JVector4F::Zero();
		private:
			float updateFactor = 0;	//-1 ~ 1
			float updateSpeed = minUpdateSpeed;
			float highLightPoint = 0;	//-1 ~ 1	cos, sin point
			float highLightRange = 0.0f;	//0 ~ 1	
		public:
			JVector4F GetColor(const int index)const noexcept;
			uint GetPointCount()const noexcept;
		public:
			void SetHighLight(const JVector4F& newHighlightColor,
				const float newHighLightPoint,
				const float newHighLightRange);
		public:
			J_SIMPLE_GET_CLAMP_SET(float, updateSpeed, UpdateSpeed, minUpdateSpeed, maxUpdateSpeed)
			J_SIMPLE_GET_SET(JVector4F, color, Color)
			J_SIMPLE_GET_SET(JVector4F, deltaColor, DeltaColor) 
		public:
			void Update();
		public:
			//scaled -1 ~ 1
			bool PushPoint(JVector2F scaledPoint);
			bool PushPoint(JVector2F unScaledPoint, const JVector2F& maxPoint);
			bool PopPoint();
		public:
			void Clear();
		};
	}
}