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