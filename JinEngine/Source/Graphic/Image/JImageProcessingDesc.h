#pragma once
#include"JImageProcessingEnum.h"
#include"../../Core/Math/JVector.h"

namespace JinEngine
{ 
	namespace Graphic
	{
		struct JBlurDesc
		{
		public:
			JVector2F imageSize;	
			J_KENEL_SIZE kernelSize;
			int mipLevel = 0;
		public:
			JBlurDesc(const JVector2F imageSize, const J_KENEL_SIZE kernelSize, const int mipLevel = 0)
				:imageSize(imageSize), kernelSize(kernelSize)
			{}
		public:
			virtual J_BLUR_TYPE GetBlurType()const noexcept = 0;
		};
		struct JBoxBlurDesc : public JBlurDesc
		{
		public:
			JBoxBlurDesc(const JVector2F imageSize, const J_KENEL_SIZE kernelSize)
				:JBlurDesc(imageSize, kernelSize)
			{}
		public:
			J_BLUR_TYPE GetBlurType()const noexcept
			{
				return J_BLUR_TYPE::BOX;
			}
		};
		struct JGaussianBlurDesc : public JBlurDesc
		{
		public:
			float sigma = 1.0f;
		public:
			JGaussianBlurDesc(const JVector2F imageSize, const J_KENEL_SIZE kernelSize, const float sigma = 1.0f)
				:JBlurDesc(imageSize, kernelSize), sigma(sigma)
			{}
		public:
			J_BLUR_TYPE GetBlurType()const noexcept
			{
				return J_BLUR_TYPE::GAUSIAAN;
			}
		};

		struct JDownSampleDesc
		{
		public:
			JVector2F imageSize;
			J_KENEL_SIZE kernelSize;
			int mipLevelCount;
		public:
			JDownSampleDesc(const JVector2F imageSize, const J_KENEL_SIZE kernelSize, const int mipLevelCount)
				:imageSize(imageSize), kernelSize(kernelSize), mipLevelCount(mipLevelCount)
			{}
		public:
			virtual J_DOWN_SAMPLING_TYPE GetDownSampleType()const noexcept = 0;
		};
		struct JBoxDownSampleDesc : public JDownSampleDesc
		{ 
		public:
			JBoxDownSampleDesc(const JVector2F imageSize, const int mipLevelCount)
				:JDownSampleDesc(imageSize, J_KENEL_SIZE::_2x2, mipLevelCount)
			{}
		public:
			J_DOWN_SAMPLING_TYPE GetDownSampleType()const noexcept final
			{
				return J_DOWN_SAMPLING_TYPE::BOX;
			}
		};
		struct JGaussianDownSampleDesc : public JDownSampleDesc
		{
		public:  
			float sigma = 1.0f;
		public:
			JGaussianDownSampleDesc(const JVector2F imageSize, const J_KENEL_SIZE kernelSize, const int mipLevelCount, const float sigma)
				:JDownSampleDesc(imageSize, kernelSize, mipLevelCount), sigma(sigma)
			{}
		public:
			J_DOWN_SAMPLING_TYPE GetDownSampleType()const noexcept final
			{
				return J_DOWN_SAMPLING_TYPE::GAUSIAAN;
			}
		};
		struct JKaiserDownSampleDesc : public JDownSampleDesc
		{
		public: 
			float beta = 1.0f;
		public:
			JKaiserDownSampleDesc(const JVector2F imageSize, const J_KENEL_SIZE kernelSize, const int mipLevelCount, const float beta)
				:JDownSampleDesc(imageSize, kernelSize, mipLevelCount), beta(beta)
			{}
		public:
			J_DOWN_SAMPLING_TYPE GetDownSampleType()const noexcept final
			{
				return J_DOWN_SAMPLING_TYPE::KAISER;
			}
		};
 
		struct JSsaoDesc
		{  
		public:
			float radius = 0.5f;
			float fadeStart = 0.2f;
			float fadeEnd = 1.0f;
			float surfaceEpsilon = 0.05f;
		public:
			J_SSAO_TYPE ssaoType = J_SSAO_TYPE::DEFAULT;
		public:
			JSsaoDesc() = default;
			JSsaoDesc(const float radius,
				const float fadeStart,
				const float fadeEnd,
				const float surfaceEpsilon)
				:radius(radius), fadeStart(fadeStart), fadeEnd(fadeEnd), surfaceEpsilon(surfaceEpsilon)
			{}
		public:
			bool operator==(const JSsaoDesc& desc)
			{
				return radius == desc.radius && 
					fadeStart == desc.fadeStart && 
					fadeEnd == desc.fadeEnd && 
					surfaceEpsilon == desc.surfaceEpsilon &&
					ssaoType == desc.ssaoType;
			}
		};  
	}
}