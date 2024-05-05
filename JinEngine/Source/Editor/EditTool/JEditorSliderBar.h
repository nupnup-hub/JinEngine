#pragma once
#include"JEditorInputBuffHelper.h"  
#include"../../Core/Unit/JManagedVariable.h" 

namespace JinEngine
{
	namespace Editor
	{
		class JEditorSliderBar
		{ 
		private: 
			Core::JRestrictedRangeVar<float> var;
		public:
			JEditorSliderBar(float value, float minValue, float maxValue);
		public:
			bool Update(const std::string& label, const float formatDigit, const bool isRight, const bool displayText);
		public:
			float GetValue()const noexcept;
		public:
			void SetValue(float value);
			void SetMinValue(float value);
			void SetMaxValue(float value);
		};
	}
}