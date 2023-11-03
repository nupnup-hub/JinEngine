#include"JBvhOption.h"
#include"../../../../JObjectFileIOHelper.h"

namespace JinEngine
{
	JBvhOption::JBvhOption(const J_ACCELERATOR_BUILD_TYPE buildType, const J_ACCELERATOR_SPLIT_TYPE splitType, const JAcceleratorOption& commonOption)
		:buildType(buildType),
		splitType(splitType),
		commonOption(commonOption)
	{}
	bool JBvhOption::EqualBvhOption(const JBvhOption& tar)const noexcept
	{
		return buildType == tar.buildType && splitType == tar.splitType;
	}
	bool JBvhOption::EqualCommonOption(const JBvhOption& tar)const noexcept
	{
		return commonOption.Equal(tar.commonOption);
	}
	void JBvhOption::Store(JFileIOTool& tool)
	{
		if (!tool.CanStore())
			return;

		tool.PushMapMember("Bvh");
		commonOption.Store(tool);
		JObjectFileIOHelper::StoreEnumData(tool, buildType, "buildType:");
		JObjectFileIOHelper::StoreEnumData(tool, splitType, "splitType:");
		tool.PopStack();
	}
	void JBvhOption::Load(JFileIOTool& tool, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid)
	{
		if (!tool.CanLoad())
			return;

		tool.PushExistStack("Bvh");
		commonOption.Load(tool, hasInnerRoot, innerRootGuid);
		JObjectFileIOHelper::LoadEnumData(tool, buildType, "buildType:");
		JObjectFileIOHelper::LoadEnumData(tool, splitType, "splitType:");
		tool.PopStack();
	}
}