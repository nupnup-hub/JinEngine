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
			//additional
			int blurCount = 1;
			bool tryBlurSubResourcr = false;
		public:
			JBlurDesc(const JVector2F imageSize, 
				const J_KERNEL_SIZE kernelSize, 
				const int mipLevel = 0,
				const int blurCount = 1,
				bool tryBlurSubResourcr = false)
				:imageSize(imageSize), kernelSize(kernelSize), mipLevel(mipLevel), blurCount(blurCount), tryBlurSubResourcr(tryBlurSubResourcr)
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
		struct JKaiserBlurDesc : public JBlurDesc
		{
		public:
			float beta = 1.0f;
		public:
			JKaiserBlurDesc(const JVector2F imageSize, const J_KERNEL_SIZE kernelSize, const float beta = 1.0f)
				:JBlurDesc(imageSize, kernelSize), beta(beta)
			{}
		public:
			J_BLUR_TYPE GetBlurType()const noexcept
			{
				return J_BLUR_TYPE::KAISER;
			}
		};

		struct JDownSampleDesc
		{
		public:
			JVector2F imageSize; 
			int mipLevelCount;
		public:
			JDownSampleDesc(const JVector2F imageSize, const int mipLevelCount)
				:imageSize(imageSize), mipLevelCount(mipLevelCount)
			{}
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

		//나중에 color curve 변경하는 기능 option 추가 할 것.
		struct JConvertColorDesc
		{
		public:
			bool reverseY = false;
		public:
			bool operator==(const JConvertColorDesc& rhs)
			{
				return reverseY == rhs.reverseY;
			}
		};
	}
}