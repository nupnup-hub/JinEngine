#include"JKdTreeOption.h"
#include"../../../../../Core/File/JFileIOHelper.h"

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
	void JKdTreeOption::Store(std::wofstream& stream)
	{
		if (!stream.is_open())
			return;

		commonOption.Store(stream);
		JFileIOHelper::StoreAtomicData(stream, L"buildType:", (int)buildType);
		JFileIOHelper::StoreAtomicData(stream, L"splitType:", (int)splitType);
	}
	void JKdTreeOption::Load(std::wifstream& stream, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid)
	{
		if (!stream.is_open() || stream.eof())
			return;

		commonOption.Load(stream, hasInnerRoot, innerRootGuid);
		int buildTypeN;
		int splitTypeN;
		JFileIOHelper::LoadAtomicData(stream, buildTypeN);
		JFileIOHelper::LoadAtomicData(stream, splitTypeN);
		buildType = (J_ACCELERATOR_BUILD_TYPE)buildTypeN;
		splitType = (J_ACCELERATOR_SPLIT_TYPE)splitTypeN;
	}
}