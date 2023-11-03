#include"JKdTreeOption.h"
#include"../../../../JObjectFileIOHelper.h"

namespace JinEngine
{
	JKdTreeOption::JKdTreeOption(const J_ACCELERATOR_BUILD_TYPE buildType,
		const J_ACCELERATOR_SPLIT_TYPE splitType,
		const JAcceleratorOption& commonOption)
		:buildType(buildType),
		splitType(splitType),
		commonOption(commonOption)
	{}
	bool JKdTreeOption::EqualKdTreeOption(const JKdTreeOption& tar)const noexcept
	{
		return buildType == tar.buildType && splitType == tar.splitType;
	}
	bool JKdTreeOption::EqualCommonOption(const JKdTreeOption& tar)const noexcept
	{
		return commonOption.Equal(tar.commonOption);
	}
	void JKdTreeOption::Store(JFileIOTool& tool)
	{
		if (!tool.CanStore())
			return;

		tool.PushMapMember("Kd-tree");
		commonOption.Store(tool);
		JObjectFileIOHelper::StoreEnumData(tool, buildType, "buildType:");
		JObjectFileIOHelper::StoreEnumData(tool, splitType, "splitType:");
		tool.PopStack();
	}
	void JKdTreeOption::Load(JFileIOTool& tool, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid)
	{
		if (!tool.CanLoad())
			return;

		tool.PushExistStack("Kd-tree");
		commonOption.Load(tool, hasInnerRoot, innerRootGuid);
		JObjectFileIOHelper::LoadEnumData(tool, buildType, "buildType:");
		JObjectFileIOHelper::LoadEnumData(tool, splitType, "splitType:");
		tool.PopStack();
	}
}