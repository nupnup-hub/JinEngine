#include"JSampler.h"
#include"../../Core/Math/JMathHelper.h"
#include<random>

namespace JinEngine::Graphic
{
	JSampler::~JSampler()
	{
		sample = nullptr;
	}
	std::vector<JVector3F> JSampler::CreateHemisphereSample(const uint maxSampleCount, float cosDensityPower)
	{  
		std::random_device rd;	//use hardware
		std::mt19937 gen(rd()); 
		std::uniform_real_distribution<float> disUNorm(0.0f, 1.0f);

		std::vector<JVector3F> sample(maxSampleCount);
		for (uint i = 0; i < maxSampleCount; ++i)
		{
			float x = disUNorm(gen);
			float y = disUNorm(gen); 

			float cosTheta = powf((1.f - y), 1.f / (cosDensityPower + 1));
			float sinTheta = sqrtf(1.f - cosTheta * cosTheta);
			sample[i].x = sinTheta * cosf(JMathHelper::Pi2 * x);
			sample[i].y = sinTheta * sinf(JMathHelper::Pi2 * x);
			sample[i].z = cosTheta;
		}
		return sample;
	} 
	void JSampler::Swap(std::unique_ptr<JGraphicBufferBase>&& newSample)
	{
		sample = std::move(newSample);
	}
}