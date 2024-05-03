#include"JSampler.h"
#include"../../Core/Math/JMathHelper.h" 
#include<numeric>

namespace JinEngine::Graphic
{ 
	JSampler::UnitSquareSample2D JSampler::GetSample2D()
	{
		return uniformSamples[GetSampleIndex()];
	}
	JSampler::HemisphereSample3D JSampler::GetHemisphereSample3D()
	{
		return hemisphereSamples[GetSampleIndex()];
	}
	uint JSampler::GetSampleSetRange()const noexcept
	{
		return numSamples;
	}
	uint JSampler::GetSampleSetCount()const noexcept
	{
		return numSampleSets;
	}
	uint JSampler::GetTotalSampleCount()const noexcept
	{
		return numSamples * numSampleSets;
	}
	uint JSampler::GetSampleIndex()
	{
		// Initialize sample and set jumps.
		if (sampleIndex % numSamples == 0)
		{
			// Pick a random index jump within a set.
			jumpIndex = GetRandomJump();

			// Pick a random set index jump.
			setJumpIndex = GetRandomSetJump() * numSamples;
		}
		return setJumpIndex + shuffledIndices[(sampleIndex++ + jumpIndex) % numSamples];
	}
	JSampler::UnitSquareSample2D JSampler::GetRandomUniform()
	{
		return JVector2F(GetRandomFloat01(), GetRandomFloat01());
	}
	uint JSampler::GetRandomNumber(uint min, uint max)
	{
		std::uniform_int_distribution<uint> distribution(min, max);
		return distribution(generatorURNG);
	}
	void JSampler::CreateByRandom()
	{
		for (auto& sample : uniformSamples)
			sample = GetRandomUniform();
	}
	// Generate multi-jittered sample patterns on a unit square [0,1].
	// Ref: Section 5.3.4 in Ray Tracing from the Ground Up.
	// The distribution has good random sampling distributions
	// with somewhat uniform distributions in both:
	// - 2D
	// - 1D projections of each axes.
	// Multi-jittered is a combination of two sample distributions:
	// - Jittered: samples are distributed on a NxN grid, 
	//             with each sample being random within its cell.
	// - N-rooks/Linear hypercube sampling: samples have uniform
	//             distribution in 1D projections of each axes.
	void JSampler::CreateByMultiJittered()
	{
		for (uint s = 0; s < numSampleSets; s++)
		{
			// Generate samples on 2 level grid, with one sample per each (x,y)
			uint sampleSetStartID = s * numSamples;

			const uint T = numSamples;
			const uint N = static_cast<uint>(sqrt(T));
 
			// Generate random samples
			for (uint col = 0, i = 0; col < N; col++)
			{
				for (uint row = 0; row < N; row++, i++)
				{
					JVector2F stratum(static_cast<float>(row), static_cast<float>(col));
					JVector2F cell(static_cast<float>(col), static_cast<float>(row));
					UnitSquareSample2D randomValue01 = GetRandomUniform();

					uniformSamples[sampleSetStartID + i].x = (randomValue01.x + cell.x) / T + stratum.x / N;
					uniformSamples[sampleSetStartID + i].y = (randomValue01.y + cell.y) / T + stratum.y / N;
				}
			}

			// Shuffle sample axes such that there's a sample in each stratum 
			// and n-rooks is maintained.

			// Shuffle x coordinate across rows within a column
			for (uint row = 0; row < N - 1; row++)
			{
				for (uint col = 0; col < N; col++)
				{
					uint k = GetRandomNumber(row + 1, N - 1);				 
					std::swap(uniformSamples[sampleSetStartID + row * N + col].x, 
						uniformSamples[sampleSetStartID + k * N + col].x);
				}
			}

			// Shuffle y coordinate across columns within a row
			for (uint row = 0; row < N; row++)
			{
				for (uint col = 0; col < N - 1; col++)
				{
					uint k = GetRandomNumber(col + 1, N - 1);
					std::swap(uniformSamples[sampleSetStartID + row * N + col].y,
						uniformSamples[sampleSetStartID + row * N + k].y);
				}
			}
		}
	}
	void JSampler::CreateHemisphereSample(float cosDensityPower)
	{
		const uint sampleCount = (uint)uniformSamples.size();
		for (uint i = 0; i < sampleCount; i++)
		{ 
			float cosTheta = powf((1.f - uniformSamples[i].y), 1.f / (cosDensityPower + 1));
			float sinTheta = sqrtf(1.f - cosTheta * cosTheta);
			hemisphereSamples[i].x = sinTheta * cosf(JMathHelper::Pi2 * uniformSamples[i].x);
			hemisphereSamples[i].y = sinTheta * sinf(JMathHelper::Pi2 * uniformSamples[i].x);
			hemisphereSamples[i].z = cosTheta;
		}
	}
	void JSampler::Reset(uint numSamples,
		uint numSampleSets,
		UniformDistributionPattern::Enum uniformPattern,
		HemisphereDistribution::Enum hemiDistribution)
	{
		sampleIndex = 0;
		numSamples = numSamples;
		numSampleSets = numSampleSets; 
		uniformSamples.resize(numSamples * numSampleSets, UnitSquareSample2D(FLT_MAX, FLT_MAX));
		shuffledIndices.resize(numSamples * numSampleSets);
		hemisphereSamples.resize(numSamples * numSampleSets, HemisphereSample3D(FLT_MAX, FLT_MAX, FLT_MAX));

		static const uint seed = 1729;
		// Reset generator and initialize distributions.
		{
			// Initialize to the same seed for determinism.
			generatorURNG.seed(seed);

			std::uniform_int_distribution<uint> jumpDistribution(0, numSamples - 1);
			std::uniform_int_distribution<uint> jumpSetDistribution(0, numSampleSets - 1);

			std::uniform_real_distribution<float> unitSquareDistribution(0.f, 1.f);

			// Specify the next representable value for the end range, since
			// uniform_real_distribution constructs excluding the end value [being, end).
			std::uniform_real_distribution<float> unitSquareDistributionInclusive(0.f, nextafter(1.f, FLT_MAX));

			GetRandomJump = bind(jumpDistribution, std::ref(generatorURNG));
			GetRandomSetJump = bind(jumpSetDistribution, std::ref(generatorURNG));
			GetRandomFloat01 = bind(unitSquareDistribution, std::ref(generatorURNG));
			GetRandomFloat01inclusive = bind(unitSquareDistributionInclusive, std::ref(generatorURNG));
		}

		// Generate random samples.
		switch (uniformPattern)
		{
		case JinEngine::Graphic::UniformDistributionPattern::Random:
			CreateByRandom();
		case JinEngine::Graphic::UniformDistributionPattern::MultiJittered:
			CreateByMultiJittered();
		default:
			return;
		}
		 
		switch (hemiDistribution)
		{
		case JinEngine::Graphic::HemisphereDistribution::Uniform:
			CreateHemisphereSample(0.0f);
		case JinEngine::Graphic::HemisphereDistribution::Cosine:
			CreateHemisphereSample(1.0f);
		default:
			return;
		}
		 
		for (uint i = 0; i < numSampleSets; i++)
		{
			auto first = std::begin(shuffledIndices) + i * numSamples;
			auto last = first + numSamples;

			std::iota(first, last, 0u); // Fill with 0, 1, ..., m_numSamples - 1 
			std::shuffle(first, last, generatorURNG);
		}
	} 
}