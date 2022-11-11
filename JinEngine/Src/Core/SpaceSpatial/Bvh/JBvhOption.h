#pragma once
#include"../JSpaceSpatialType.h"
#include"../JSpaceSpatialOption.h"
namespace JinEngine
{
	namespace Core
	{
		struct JBvhOption
		{
		public:
			J_SPACE_SPATIAL_BUILD_TYPE buildType = J_SPACE_SPATIAL_BUILD_TYPE::TOP_DOWN;
			J_SPACE_SPATIAL_SPLIT_TYPE splitType = J_SPACE_SPATIAL_SPLIT_TYPE::SAH;
		public:
			JSpaceSpatialOption commonOption;
		public:
			JBvhOption() = default;
			JBvhOption(const J_SPACE_SPATIAL_BUILD_TYPE buildType, const J_SPACE_SPATIAL_SPLIT_TYPE splitType, const JSpaceSpatialOption& commonOption);
		public:
			bool EqualBvhOption(const JBvhOption& tar)const noexcept;
			bool EqualCommonOption(const JBvhOption& tar)const noexcept;
		public:
			void Store(std::wofstream& stream);
			void Load(std::wifstream& stream, _Out_ bool& hasInnerRoot, _Out_ size_t& innerRootGuid);
		};
	}
}