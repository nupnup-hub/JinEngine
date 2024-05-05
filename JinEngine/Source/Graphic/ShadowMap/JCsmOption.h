#pragma once
#include"../../Core/JCoreEssential.h"  
#include"../../Core/Utility/JMacroUtility.h" 

namespace JinEngine
{
	namespace Graphic
	{
		struct JCsmOption
		{
		public:
			static constexpr uint minCountOfSplit = 1;
			static constexpr uint maxCountOfSplit = 8;
			static constexpr float minSplitRate = 0.01f;
			static constexpr float maxSplitRate = 1.0f;  
			static constexpr float minLevelRate = 0.01f;
			static constexpr float maxLevelRate = 1.0f;
		private: 
			// [0.0 ~ 1.0]	final split rate = blend * logSplit + (1 - blend) * fixedRate
			float splitBlendRate = 0.75f;
			uint splitCount = 4;
		private:
			float shadowDistance = 1000; 
		private:
			/**
			* [0.0 ~ 1.0] blend n ~ n + 1 cascade (if n != max index)
			* [1 - levelBlendRate] blend start 
			* [1] blend end
			*/
			float levelBlendRate = 0.1f;
		public:
			bool operator=(const JCsmOption& rhs)const noexcept;
		public: 
			uint GetMaxSplitCount()const noexcept;
			uint GetSplitCount()const noexcept;
			float GetSplitBlendRate()const noexcept;
			float GetShadowDistance()const noexcept; 
			float GetLevelBlendRate()const noexcept;
		public:
			void SetSplitCount(const uint newCount)noexcept;
			/*
			* @brief final split rate = blend * logSplit + (1 - blend) * fixedRate
			* @param range [0.0 ~ 1.0]
			*/
			void SetSplitBlendRate(const float value)noexcept;
			void SetShadowDistance(const float value)noexcept;
			void SetLevelBlendRate(const float value)noexcept; 
		};
	}
}