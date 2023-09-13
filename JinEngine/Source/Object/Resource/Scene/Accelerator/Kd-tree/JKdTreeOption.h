#pragma once
#include"../JAcceleratorType.h"
#include"../JAcceleratorOption.h"
 
namespace JinEngine
{
	struct JKdTreeOption
	{
	public:
		J_ACCELERATOR_BUILD_TYPE buildType = J_ACCELERATOR_BUILD_TYPE::TOP_DOWN;
		J_ACCELERATOR_SPLIT_TYPE splitType = J_ACCELERATOR_SPLIT_TYPE::SAH;
	public:
		JAcceleratorOption commonOption;
	public:
		JKdTreeOption() = default;
		JKdTreeOption(const J_ACCELERATOR_BUILD_TYPE buildType,
			const J_ACCELERATOR_SPLIT_TYPE splitType,
			const JAcceleratorOption& commonOption);
	public:
		bool EqualKdTreeOption(const JKdTreeOption& tar)const noexcept;
		bool EqualCommonOption(const JKdTreeOption& tar)const noexcept;
	public:
		void Store(std::wofstream& stream);
		void Load(std::wifstream& stream, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid);
	};
}