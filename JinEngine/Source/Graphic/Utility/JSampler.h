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
#include<random>
namespace JinEngine
{
	namespace Graphic
	{
		namespace HemisphereDistribution 
		{
			enum Enum 
			{
				Uniform,
				Cosine
			}; 
		}
		namespace UniformDistributionPattern
		{
			enum Enum
			{
				Random,
				MultiJittered
			};
		}
		class JSampler
		{  
		public:
			using UnitSquareSample2D = JVector2F;  // unit square sample with a valid range of <0,1>
			using HemisphereSample3D = JVector3F;  // hemisphere sample <0,1>
		private:
			std::function<uint()> GetRandomJump;        // Generates a random uniform index within [0, m_numSamples - 1]
			std::function<uint()> GetRandomSetJump;     // Generates a random uniform index within [0, m_numSampleSets - 1]
			std::function<float()> GetRandomFloat01;    // Generates a random uniform float within [0,1)
			std::function<float()> GetRandomFloat01inclusive; // Generates a random uniform float within [0,1]
		private:
			std::vector<UnitSquareSample2D> uniformSamples;
			std::vector<HemisphereSample3D> hemisphereSamples;
			std::vector<uint> shuffledIndices;
		private:
			std::mt19937 generatorURNG;  // Uniform random number generator
			uint numSamples = 0;
			uint numSampleSets = 0;
			uint sampleIndex = 0;
			uint jumpIndex = 0;
			uint setJumpIndex = 0;
		public:
			UnitSquareSample2D GetSample2D();
			HemisphereSample3D GetHemisphereSample3D();
			uint GetSampleSetRange()const noexcept;
			uint GetSampleSetCount()const noexcept;
			uint GetTotalSampleCount()const noexcept;
		private:
			uint GetSampleIndex();
			UnitSquareSample2D GetRandomUniform();
			uint GetRandomNumber(uint min, uint max);
		private:
			void CreateByRandom();
			void CreateByMultiJittered();
			void CreateHemisphereSample(float cosDensityPower);
		public:
			void Reset(uint numSamples, 
				uint numSampleSets,
				UniformDistributionPattern::Enum uniformPattern,
				HemisphereDistribution::Enum hemiDistribution);
		};
	}
}