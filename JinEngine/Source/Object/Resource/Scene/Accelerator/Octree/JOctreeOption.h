#pragma once
#include"../JAcceleratorOption.h"

namespace JinEngine
{
	struct JOctreeOption
	{
	public:
		uint minSize = 32;
		uint octreeSizeSquare = 9;
		float looseFactor = 2;
	public:
		JAcceleratorOption commonOption;
	public:
		JOctreeOption() = default;
		JOctreeOption(const uint minSize, const uint octreeSizeSquare, const float looseFactor, const JAcceleratorOption& commonOption);
	public:
		bool EqualOctreeOption(const JOctreeOption& tar)const noexcept;
		bool EqualCommonOption(const JOctreeOption& tar)const noexcept;
	public:
		void Store(std::wofstream& stream);
		void Load(std::wifstream& stream, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid);
	};
}