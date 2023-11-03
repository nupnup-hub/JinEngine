#pragma once
#include"JBlurEnum.h"
#include"../../Core/Math/JVector.h"

namespace JinEngine
{
	namespace Graphic
	{
		struct JBlurDesc
		{
		public:
			JVector2F imageSize;	
			J_BLUR_KENEL_SIZE kenelSize;
		public:
			JBlurDesc(const JVector2F imageSize, const J_BLUR_KENEL_SIZE kenelSize)
				:imageSize(imageSize), kenelSize(kenelSize)
			{}
		public:
			virtual J_BLUR_TYPE GetBlurType()const noexcept = 0;
		};

		struct JBoxBlurDesc : public JBlurDesc
		{
		public:
			JBoxBlurDesc(const JVector2F imageSize, const J_BLUR_KENEL_SIZE kenelSize)
				:JBlurDesc(imageSize, kenelSize)
			{}
		public:
			J_BLUR_TYPE GetBlurType()const noexcept
			{
				return J_BLUR_TYPE::BOX;
			}
		};
		struct JGausiaanBlurDesc : public JBlurDesc
		{
		public:
			float sigma = 1.0f;
		public:
			JGausiaanBlurDesc(const JVector2F imageSize, const J_BLUR_KENEL_SIZE kenelSize, const float sigma = 1.0f)
				:JBlurDesc(imageSize, kenelSize), sigma(sigma)
			{}
		public:
			J_BLUR_TYPE GetBlurType()const noexcept
			{
				return J_BLUR_TYPE::GAUSIAAN;
			}
		};
	}
}