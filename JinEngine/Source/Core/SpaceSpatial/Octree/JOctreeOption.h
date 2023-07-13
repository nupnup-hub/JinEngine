#pragma once
#include"../../JDataType.h"
#include"../JSpaceSpatialOption.h"
namespace JinEngine
{
	namespace Core
	{
		struct JOctreeOption
		{
		public:
			uint minSize = 32;
			uint octreeSizeSquare = 9;
			float looseFactor = 2; 
		public:
			JSpaceSpatialOption commonOption;
		public:
			JOctreeOption() = default;
			JOctreeOption(const uint minSize, const uint octreeSizeSquare, const float looseFactor, const JSpaceSpatialOption& commonOption);
		public:
			bool EqualOctreeOption(const JOctreeOption& tar)const noexcept;
			bool EqualCommonOption(const JOctreeOption& tar)const noexcept;
		public:
			void Store(std::wofstream& stream);
			void Load(std::wifstream& stream, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid);
		};
	}
}