#include"JOctreeOption.h"
#include"../../../Core/File/JFileIOHelper.h"

namespace JinEngine
{
	namespace Core
	{
		JOctreeOption::JOctreeOption(const uint minSize, const uint octreeSizeSquare, const float looseFactor, const JSpaceSpatialOption& commonOption)
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
			JFileIOHelper::StoreAtomicData(stream, L"minsize:", minSize);
			JFileIOHelper::StoreAtomicData(stream, L"octreeSizeSquare:", octreeSizeSquare);
			JFileIOHelper::StoreAtomicData(stream, L"looseFactor:", looseFactor);
		}
		void JOctreeOption::Load(std::wifstream& stream, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid)
		{
			if (!stream.is_open() || stream.eof())
				return;

			commonOption.Load(stream, hasInnerRoot, innerRootGuid);
			JFileIOHelper::LoadAtomicData(stream, minSize);
			JFileIOHelper::LoadAtomicData(stream, octreeSizeSquare);
			JFileIOHelper::LoadAtomicData(stream, looseFactor);
		}
	}
}