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
		std::vector<JAnimationSample>animationSample;
		size_t skeletonHash;
		uint32 clipLength;
		float framePerSecond;
	public:
		JAnimationData(Core::JFbxAnimationData&& jfbxData);
	};
}