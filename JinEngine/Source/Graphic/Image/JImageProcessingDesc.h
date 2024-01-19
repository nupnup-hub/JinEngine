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
			J_KERNEL_SIZE kernelSize;
			int mipLevel = 0;
		public:
			JBlurDesc(const JVector2F imageSize, const J_KERNEL_SIZE kernelSize, const int mipLevel = 0)
				:imageSize(imageSize), kernelSize(kernelSize)
			{}
		public:
			virtual J_BLUR_TYPE GetBlurType()const noexcept = 0;
		};
		struct JBoxBlurDesc : public JBlurDesc
		{
		public:
			JBoxBlurDesc(const JVector2F imageSize, const J_KERNEL_SIZE kernelSize)
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
			JGaussianBlurDesc(const JVector2F imageSize, const J_KERNEL_SIZE kernelSize, const float sigma = 1.0f)
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
			J_KERNEL_SIZE kernelSize;
			int mipLevelCount;
		public:
			JDownSampleDesc(const JVector2F imageSize, const J_KERNEL_SIZE kernelSize, const int mipLevelCount)
				:imageSize(imageSize), kernelSize(kernelSize), mipLevelCount(mipLevelCount)
			{}
		public:
			virtual J_DOWN_SAMPLING_TYPE GetDownSampleType()const noexcept = 0;
		};
		struct JBoxDownSampleDesc : public JDownSampleDesc
		{ 
		public:
			JBoxDownSampleDesc(const JVector2F imageSize, const int mipLevelCount)
				:JDownSampleDesc(imageSize, J_KERNEL_SIZE::_3x3, mipLevelCount)
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
			JGaussianDownSampleDesc(const JVector2F imageSize, const J_KERNEL_SIZE kernelSize, const int mipLevelCount, const float sigma)
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
			JKaiserDownSampleDesc(const JVector2F imageSize, const J_KERNEL_SIZE kernelSize, const int mipLevelCount, const float beta)
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
			float bias = 0.0f;
			float sharpness = 1.0f;
		public:
			//hbad
			float smallAoScale = 0.6f;
			float largeAoScale = 0.125f;
		public:
			bool useBlur = true;
		public:
			J_KERNEL_SIZE blurKenelSize = J_KERNEL_SIZE::_3x3;
		public:
			J_SSAO_TYPE ssaoType = J_SSAO_TYPE::DEFAULT;
			J_SSAO_SAMPLE_TYPE sampleType = J_SSAO_SAMPLE_TYPE::NORMAL;
			J_SSAO_BLUR_TYPE blurType = J_SSAO_BLUR_TYPE::BILATERAL;
		public:
			JSsaoDesc() = default;
			JSsaoDesc(const float radius, const float bias)
				:radius(radius), bias(bias)
			{}
		public:
			bool operator==(const JSsaoDesc& desc)
			{
				return radius == desc.radius &&
					bias == desc.bias &&
					sharpness == desc.sharpness &&
					blurKenelSize == desc.blurKenelSize &&
					ssaoType == desc.ssaoType &&
					sampleType == desc.sampleType &&
					blurType == desc.blurType &&
					smallAoScale == desc.smallAoScale && 
					largeAoScale == desc.largeAoScale;
			}
		};  
	}
}