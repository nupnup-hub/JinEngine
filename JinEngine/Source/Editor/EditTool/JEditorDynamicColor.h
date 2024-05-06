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