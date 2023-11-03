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
	void JOctreeOption::Store(JFileIOTool& tool)
	{
		if (!tool.CanStore())
			return;

		tool.PushMapMember("Occtree");
		commonOption.Store(tool);
		JObjectFileIOHelper::StoreAtomicData(tool, minSize, "minsize:");
		JObjectFileIOHelper::StoreAtomicData(tool, octreeSizeSquare, "octreeSizeSquare:");
		JObjectFileIOHelper::StoreAtomicData(tool, looseFactor, "looseFactor:");
		tool.PopStack();
	}
	void JOctreeOption::Load(JFileIOTool& tool, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid)
	{
		if (!tool.CanLoad())
			return;

		tool.PushExistStack("Occtree");
		commonOption.Load(tool, hasInnerRoot, innerRootGuid);
		JObjectFileIOHelper::LoadAtomicData(tool, minSize);
		JObjectFileIOHelper::LoadAtomicData(tool, octreeSizeSquare);
		JObjectFileIOHelper::LoadAtomicData(tool, looseFactor);
		tool.PopStack();
	}
}
