#include"JOctreeOption.h"
#include"../../../../JObjectFileIOHelper.h"

namespace JinEngine
{
	JOctreeOption::JOctreeOption(const uint minSize, const uint octreeSizeSquare, const float looseFactor, const JAcceleratorOption& commonOption)
		:minSize(minSize),
		octreeSizeSquare(octreeSizeSquare),
		looseFactor(looseFactor),
		commonOption(commonOption)
	{}
	bool JOctreeOption::EqualOctreeOption(const JOctreeOption& tar)const noexcept
	{
		return minSize == tar.minSize &&
			octreeSizeSquare == tar.octreeSizeSquare &&
			looseFactor == tar.looseFactor;
	}
	bool JOctreeOption::EqualCommonOption(const JOctreeOption& tar)const noexcept
	{
		return commonOption.Equal(tar.commonOption);
	}
	void JOctreeOption::Store(std::wofstream& stream)
	{
		if (!stream.is_open())
			return;

		commonOption.Store(stream);
		JObjectFileIOHelper::StoreAtomicData(stream, L"minsize:", minSize);
		JObjectFileIOHelper::StoreAtomicData(stream, L"octreeSizeSquare:", octreeSizeSquare);
		JObjectFileIOHelper::StoreAtomicData(stream, L"looseFactor:", looseFactor);
	}
	void JOctreeOption::Load(std::wifstream& stream, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid)
	{
		if (!stream.is_open() || stream.eof())
			return;

		commonOption.Load(stream, hasInnerRoot, innerRootGuid);
		JObjectFileIOHelper::LoadAtomicData(stream, minSize);
		JObjectFileIOHelper::LoadAtomicData(stream, octreeSizeSquare);
		JObjectFileIOHelper::LoadAtomicData(stream, looseFactor);
	}
}
