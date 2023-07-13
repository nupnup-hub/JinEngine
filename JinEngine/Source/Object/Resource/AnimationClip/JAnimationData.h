#pragma once
#include<vector>
#include"JAnimationSample.h" 
#include"../../../Core/JDataType.h"

namespace JinEngine
{
	namespace Core
	{
		struct JFbxAnimationData;
	}

	struct JAnimationData
	{
	public:
		//total count is same as base skeletonAsset joint
		std::vector<JAnimationSample>animationSample;
		size_t skeletonHash;
		uint32 length;
		float framePerSecond;
	public:
		JAnimationData() = default;
		JAnimationData(Core::JFbxAnimationData&& jfbxData);
	};
}